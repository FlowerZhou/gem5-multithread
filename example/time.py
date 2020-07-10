#! /usr/bin/env python
import time
from time import strftime,gmtime
strftime("%m/%d/%Y %H:%M")
print(time.strftime("%Y-%m-%d-%H_%M_%S", time.localtime()))
