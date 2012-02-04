#!/bin/sh

#  bootstrap_language.sh
#  TameParse
#
#  Created by Andrew Hunter on 03/09/2011.
#  Copyright 2011-2012 Andrew Hunter. All rights reserved.

echo Building tameparse_language.*
echo Writing to ${BUILT_PRODUCTS_DIR}

cd "${BUILT_PRODUCTS_DIR}"
${BUILT_PRODUCTS_DIR}/bootstrap
