//
//  parser_stack.h
//  Parse
//
//  Created by Andrew Hunter on 08/05/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
//

#ifndef _LR_PARSER_STACK_H
#define _LR_PARSER_STACK_H

#include <cstdlib>

#include <vector>
#include <stack>

namespace lr {
    ///
    /// \brief Class representing the parser stack
    ///
    /// The design of a parser stack presents an interesting problem: the simplest design is just a list of
    /// parser states. This is fairly useless in practice as it can only be used to determine whether or not
    /// a string matches a particular language, and can't return any useful data.
    ///
    /// A more useful stack contains an extra element: a storage location representing the data associated with
    /// the item that was matched at a particular location. This allows a parser to return a data structure
    /// representing what was matched by the parser, and is what makes parsers useful in compiler design.
    ///
    /// Both these designs have a disadvantage: there is no way to rewind the parser to a previous state. This
    /// stack implementation works around this by using garbage collection: stack items are not removed until
    /// the stack is full, and a simple mark-and-sweep algorithm is used to remove existing elements.
    ///
    /// This allows for parsers that can backtrack to a previous state without the need to replicate the entire
    /// stack. A mark-and-sweep approach to freeing stack space has much less overhead than a reference counting
    /// approach (in a garbage collected language, a linked list is all that's required). This also makes it
    /// possible to implement GLR parsers that evaluate all possible states in parallel, and to perform error
    /// recovery by running the parser in a speculative manner.
    ///
    /// Our default parser implementation is deterministic (neither GLR nor backtracking), so these advantages
    /// are perhaps not immediately obvious: however, for complex languages or cases where good error recovery
    /// is important, this ability can be a major advantage.
    ///
    /// The item_type used with this class must have a default constructor, and support assignment and copying.
    /// You don't actually create an instance of the stack class, but rather the parser_stack::reference class.
    /// The stack is free once there are no more references to it.
    ///
    template<typename item_type, int initial_depth = 64> class parser_stack {
    public:
        class entry;
        
    private:
        class internal_stack;
        
        friend class internal_stack;

        typedef parser_stack<item_type, initial_depth> pstack;

    public:
        ///
        /// \brief Structure representing an entry in a parser stack
        ///
        class entry {
        private:
            friend class parser_stack<item_type, initial_depth>;
            friend class parser_stack<item_type, initial_depth>::internal_stack;
            
            /// \brief -1 for a 'head' element, -2 for an empty element, or the entry 'below' this one
            int m_PreviousIndex;
            
        public:
            static const int head = -1;
            static const int empty = -2;
            
            inline entry()
            : m_PreviousIndex(empty) {
            }
            
            /// \brief The item associated with this entry
            item_type item;
            
            /// \brief The state ID associated with this entry
            int state;
        };
        
    private:
        class internal_stack {
            friend class parser_stack<item_type, initial_depth>;
            
            /// \brief The first stack reference known about by this stack
            parser_stack<item_type, initial_depth>* m_RootReference;
            
            /// \brief A vector of entries in this stack
            std::vector<entry> m_Stack;
            
            /// \brief The first unused stack entry
            int m_FirstUnused;
            
            /// \brief The number of free entries in the stack
            int m_NumFree;
            
            internal_stack(const internal_stack& copyFrom);
            
            /// \brief Disabled assignment
            internal_stack& operator=(const internal_stack& assignFrom);
            
        public:
            /// \brief Creates a new stack
            internal_stack()
            : m_RootReference(NULL) {
                m_Stack.resize(initial_depth);
                m_FirstUnused = 0;
                m_NumFree = (int)m_Stack.size();
            }

            /// \brief Destroys the stack (and any remaining references)
            ~internal_stack() {
                // Delete any remaining references
                while (m_RootReference != NULL) {
                    delete m_RootReference;
                }
            }
            
            /// \brief Garbage collects this stack
            void collect() {
                // Create a vector of marked elements
                std::vector<bool> marks;
                marks.resize(m_Stack.size(), false);
                
                // Mark any entries used by an active reference
                std::stack<int> waiting;
                
                pstack* ref = m_RootReference;
                while (ref != NULL) {
                    waiting.push(ref->m_Index);
                    ref = ref->m_Next;
                }
                
                // Mark any items that are in use
                while (!waiting.empty()) {
                    // Get the next item to process
                    int next = waiting.top();
                    waiting.pop();
                    
                    // Mark this item as being in-use
                    marks[next] = true;
                    
                    // Push the preceeding entry if there is one
                    entry& waitingEntry = m_Stack[next];
                    if (waitingEntry.m_PreviousIndex >= 0) {
                        if (!marks[waitingEntry.m_PreviousIndex]) {
                            waiting.push(waitingEntry.m_PreviousIndex);
                        }
                    }
                }
                
                // Sweep any unused items in the stack
                m_NumFree = 0;
                for (size_t x=0; x<m_Stack.size(); ++x) {
                    if (!marks[x]) {
                        m_Stack[x].m_PreviousIndex = entry::empty;
                        ++m_NumFree;
                    }
                }
            }
            
        private:
            void grow_stack() {
                // Work out how many new items to create
                int numNew = (int)m_Stack.size();
                if (numNew > initial_depth * 8) numNew = initial_depth * 8;
                
                // Resize the stack by this amount
                m_Stack.resize(m_Stack.size() + numNew);
                m_NumFree += numNew;
            }
            
        public:
            /// \brief Finds the next unused item
            int get_new() {
                // Collect if we've run out of free items, and grow the stack if it's still looking empty
                if (m_NumFree <= 0) {
                    collect();
                    if (m_NumFree < initial_depth/2) {
                        grow_stack();
                    }
                }
                
                // Find a free entry
                while (m_Stack[m_FirstUnused].m_PreviousIndex != entry::empty) {
                    ++m_FirstUnused;
                    if (m_FirstUnused >= (int) m_Stack.size()) m_FirstUnused = 0;
                }
                
                // m_FirstUnused now points to an entry we can use
                int result = m_FirstUnused;
                ++m_FirstUnused;
                if (m_FirstUnused >= (int) m_Stack.size()) m_FirstUnused = 0;
                
                // Make this a 'head' entry
                m_Stack[result].m_PreviousIndex = entry::head;
                
                // Number of free entries goes down
                m_NumFree--;
                
                return result;
            }
        };
        
    private:
        
        /// \brief The stack that owns this reference
        internal_stack* m_Stack;
        
        /// \brief The index of this reference
        int m_Index;
        
        /// \brief The previous reference in the list of references
        pstack* m_Last;
        
        /// \brief The next reference in the list of references
        pstack* m_Next;
        
    private:
        parser_stack(pstack* stack, int index)
        : m_Index(index)
        , m_Stack(stack) {
            m_Next = m_Stack->m_RootReference;
            m_Last = NULL;
            m_Stack->m_RootReference = this;
        }
        
    public:
        parser_stack() 
        : m_Stack(new internal_stack())
        , m_Index(m_Stack->get_new()) {
            m_Next = m_Stack->m_RootReference;
            m_Last = NULL;
            m_Stack->m_RootReference = this;
        }
        
        /// \brief Creates a copy of a particular reference
        inline parser_stack(const parser_stack& copyFrom)
        : m_Stack(copyFrom.m_Stack)
        , m_Index(copyFrom.m_Index) {
            m_Next = m_Stack->m_RootReference;
            m_Last = NULL;
            m_Stack->m_RootReference = this;
        }
        
        /// \brief Assignment operator
        inline parser_stack& operator=(const parser_stack& copyFrom) {
            // This will only work for references from the same stack
            m_Index = copyFrom.m_Index;
            
            return *this;
        }
        
        /// \brief Destructor
        inline ~parser_stack() {
            // Remove this reference from the list of references
            if (m_Next) {
                m_Next->m_Last = m_Last;
            }
            if (m_Last) {
                m_Last->m_Next = m_Next;
            } else {
                m_Stack->m_RootReference = m_Next;
            }
            
            if (m_Stack->m_RootReference == NULL) {
                delete m_Stack;
            }
        }
        
        inline entry& operator*() {
            return m_Stack->m_Stack[m_Index];
        }
        
        inline entry* operator->() {
            return &m_Stack->m_Stack[m_Index];
        }

        inline const entry& operator*() const {
            return m_Stack->m_Stack[m_Index];
        }
        
        inline const entry* operator->() const {
            return &m_Stack->m_Stack[m_Index];
        }
        
        /// \brief Returns the entry at the specified offset from this entry. 'x' should be a negative value
        ///
        /// IE, reference[-1] gives the entry preceeding this one on the stack
        inline entry& operator[](int x) {
            int index = m_Index;
            for (int pos = x; pos < 0; ++pos) {
                int nextIndex = m_Stack->m_Stack[index].m_PreviousIndex;
                if (nextIndex >= 0) index = nextIndex;
            }
            return m_Stack->m_Stack[index];
        }
        
        /// \brief Returns the entry at the specified offset from this entry. 'x' should be a negative value
        ///
        /// IE, reference[-1] gives the entry preceeding this one on the stack
        inline const entry& operator[](int x) const {
            int index = m_Index;
            for (int pos = x; pos < 0; ++pos) {
                int nextIndex = m_Stack->m_Stack[index].m_PreviousIndex;
                if (nextIndex >= 0) index = nextIndex;
            }
            return m_Stack->m_Stack[index];
        }

        /// \brief Pushes a new item onto the stack, and updates this to point at it
        inline void push(int state, const item_type& newItem) {
            int newIndex = m_Stack->get_new();
            
            entry& newEntry = m_Stack->m_Stack[newIndex];
            
            newEntry.state              = state;
            newEntry.item               = newItem;
            newEntry.m_PreviousIndex    = m_Index;
            
            m_Index = newIndex;
        }
        
        /// \brief Pops an item from the stack (returns false if this is currently pointing at a head item)
        ///
        /// This reference is adjusted to point at the new head of the stack
        inline bool pop() {
            entry& ourEntry = operator*();
            if (ourEntry.m_PreviousIndex == -1) return false;
            m_Index = ourEntry.m_PreviousIndex;
            return true;
        }
    };
}

#endif
