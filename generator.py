# a simple string generator for creating test data

import string
from random import choice

chars = string.letters + string.digits

f = open('test', 'wb')

for i in range(1, 4000000):
	f.write(''.join(choice(chars) for _ in range(5)))

f.close()
