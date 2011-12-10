#!/bin/sh

#  build_defn.sh
#  TameParse
#
#  Created by Andrew Hunter on 27/06/2011.
#  Copyright 2011 _Andrew Hunter_. All rights reserved.

echo Building definition_tp.h
echo Writing to ${BUILT_PRODUCTS_DIR}/definition_tp.h

cd "${SRCROOT}/TameParse/Language"
xxd -i "definition.tp" >"${BUILT_PRODUCTS_DIR}/definition_tp.h"
