#!/bin/sh

#  build_defn.sh
#  TameParse
#
#  Created by Andrew Hunter on 27/06/2011.
#  Copyright 2011 __MyCompanyName__. All rights reserved.

echo Building definition_txt.h
echo Writing to ${BUILT_PRODUCTS_DIR}/definition_txt.h

cd "${SRCROOT}/TameParse/Language"
xxd -i "definition.txt" >"${BUILT_PRODUCTS_DIR}/definition_txt.h"
