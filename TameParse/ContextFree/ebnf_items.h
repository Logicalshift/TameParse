//
//  ebnf_items.h
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  
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

#ifndef _CONTEXTFREE_EBNF_ITEMS_H
#define _CONTEXTFREE_EBNF_ITEMS_H

#include <vector>

#include "TameParse/ContextFree/item.h"
#include "TameParse/Util/container.h"

namespace contextfree {
    /// \brief Forward declaration of a rule
    class rule;
    
    /// \brief Class that can contain a reference to a rule
    typedef util::container<rule> rule_container;

    /// \brief List of rules
    typedef std::vector<rule_container> rule_list;

    ///
    /// \brief Abstract base class of an EBNF item
    ///
    /// Extended Backus-Naur Form extends plain old BNF by adding constructs that nest rules. Subclasses of this class represent
    /// these constructs.
    ///
    class ebnf : public item {
    private:
        /// \brief The rules that are included in this item (by default there is 1, which is initialised as being empty)
        rule_list* m_Rules;
        
        /// \brief Disabled assignment
        ebnf& operator=(const ebnf& assignFrom);

    protected:
        /// \brief Adds a new rule to this construction
        void add_rule(const rule& newRule);
        
        /// \brief Copy constructor
        ebnf(const ebnf& copyFrom);
        
        /// \brief Copy constructor
        ebnf(const rule_list& copyFrom);
        
        /// \brief The list of rules in this item
        inline const rule_list& rules() const { return *m_Rules; }
        
    public:
        /// \brief Creates a new EBNF item
        ebnf();
        
        /// \brief Creates a new EBNF item from a rule
        ebnf(const rule& copyFrom);
        
        /// \brief Destructor
        virtual ~ebnf();
        
        /// \brief Creates a clone of this item
        virtual item* clone() const = 0;

        /// \brief Compares this item to another. Returns true if they are the same
        virtual bool operator==(const item& compareTo) const;
        
        /// \brief Orders this item relative to another item
        virtual bool operator<(const item& compareTo) const;
        
        /// \brief Returns NULL, or this item as an EBNF item
        virtual const class ebnf* cast_ebnf() const;

        /// \brief The rule that defines this item
        rule_container& get_rule();
        
        /// \brief The rule that defines this item
        const rule_container& get_rule() const;
        
        /// \brief Type defining an iterator through the rules contained in this item
        typedef rule_list::const_iterator rule_iterator;
        
        /// \brief The number of alternative rules within this item (most items have only one)
        inline size_t count_rules() const { return m_Rules->size(); }

        /// \brief The first rule in this item
        inline rule_iterator first_rule() const { return m_Rules->begin(); }
        
        /// \brief The rule after the last rule in this item
        inline rule_iterator last_rule() const { return m_Rules->end(); }
    };
    
    ///
    /// \brief EBNF rule representing an optional section
    ///
    class ebnf_optional : public ebnf {
    protected:
        inline ebnf_optional(const ebnf_optional& copyFrom) : ebnf(copyFrom) { }
        
    public:
        inline ebnf_optional() : ebnf() { }
        inline ebnf_optional(const rule& optionalRule) : ebnf(optionalRule) { }
        
        /// \brief Creates a clone of this item
        virtual item* clone() const;
        
        /// \brief The type of this item
        virtual kind type() const;
        
        /// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
        ///
        /// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
        /// set for the rules that make them up.
        ///
        /// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
        /// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
        virtual item_set first(const grammar& gram) const;
        
        /// \brief Computes the closure of this rule in the specified grammar
        virtual void closure(const lr::lr1_item& item, lr::lr1_item_set& state, const grammar& gram) const;
    };
    
    ///
    /// \brief EBNF rule representing a section that repeats one or more times (combine with ebnf_optional to get a section that repeats 0 or more times)
    ///
    class ebnf_repeating : public ebnf {
    protected:
        inline ebnf_repeating(const ebnf_repeating& copyFrom) : ebnf(copyFrom) { }

    public:
        inline ebnf_repeating() : ebnf() { }
        inline ebnf_repeating(const rule& repeatingRule) : ebnf(repeatingRule) { }
        
        /// \brief Creates a clone of this item
        virtual item* clone() const;
        
        /// \brief The type of this item
        virtual kind type() const;
        
        /// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
        ///
        /// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
        /// set for the rules that make them up.
        ///
        /// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
        /// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
        virtual item_set first(const grammar& gram) const;
        
        /// \brief Computes the closure of this rule in the specified grammar
        virtual void closure(const lr::lr1_item& item, lr::lr1_item_set& state, const grammar& gram) const;
    };
    
    ///
    /// \brief EBNF rule representing a section that repeats zero or more times (combine with ebnf_optional to get a section that repeats 0 or more times)
    ///
    class ebnf_repeating_optional : public ebnf {
    protected:
        inline ebnf_repeating_optional(const ebnf_repeating_optional& copyFrom) : ebnf(copyFrom) { }
        
    public:
        inline ebnf_repeating_optional() : ebnf() { }
        inline ebnf_repeating_optional(const rule& repeatingRule) : ebnf(repeatingRule) { }
        
        /// \brief Creates a clone of this item
        virtual item* clone() const;
        
        /// \brief The type of this item
        virtual kind type() const;
        
        /// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
        ///
        /// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
        /// set for the rules that make them up.
        ///
        /// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
        /// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
        virtual item_set first(const grammar& gram) const;
        
        /// \brief Computes the closure of this rule in the specified grammar
        virtual void closure(const lr::lr1_item& item, lr::lr1_item_set& state, const grammar& gram) const;
    };
    
    ///
    /// \brief EBNF rule representing a section with one or more alternatives
    ///
    class ebnf_alternate : public ebnf {
    protected:
        inline ebnf_alternate(const ebnf_alternate& copyFrom) : ebnf(copyFrom) { }

    public:
        inline ebnf_alternate() : ebnf() { }
        inline ebnf_alternate(const rule& firstRule) : ebnf(firstRule) { }
        
        /// \brief Adds a new rule
        rule_container add_rule();
        
        /// \brief Creates a clone of this item
        virtual item* clone() const;
        
        /// \brief The type of this item
        virtual kind type() const;
        
        /// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
        ///
        /// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
        /// set for the rules that make them up.
        ///
        /// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
        /// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
        virtual item_set first(const grammar& gram) const;
        
        /// \brief Computes the closure of this rule in the specified grammar
        virtual void closure(const lr::lr1_item& item, lr::lr1_item_set& state, const grammar& gram) const;
    };
}

#endif
