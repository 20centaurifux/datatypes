define(`PREFIX', `rbtree')
define(`TYPENAME', `RBTree')
define(`NEW_STR_ARGS', `str_compare, free, NULL, NULL')
define(`NEW_INT_ARGS', `direct_compare, NULL, NULL, NULL')
define(`HAS_ITER_FREE_FUNC', `true')
include(`test-map.m4')
