#!/bin/sh

#  build_defn.sh
#  Parse
#
#  Created by Andrew Hunter on 27/06/2011.
#  Copyright 2011 __MyCompanyName__. All rights reserved.

echo Building definition_txt.h
echo Writing to ${BUILT_PRODUCTS_DIR}/definition_txt.h

cd "${SRCROOT}/Language"
xxd -i "definition.txt" >"${BUILT_PRODUCTS_DIR}/definition_txt.h"
