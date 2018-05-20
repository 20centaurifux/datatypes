define(`PREFIX', `hashtable')
define(`TYPENAME', `HashTable')
define(`NEW_ARGS', `HASHTABLE_AUTO_RESIZE, str_hash, str_equal, free, NULL')
include(`test-map.m4')
