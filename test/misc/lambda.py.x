﻿#
# Copyright (C) 2024 The XLang Foundation
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# <END>

def dbg_test():
  print("in dbg_test 01")
  print("--------in dbg_test 01--")

dbg_test()

x=10
y=100
z =x+y
f = (x:int,y:int,z:int){
    f2 =(k:int,l)
    {
       print("      inside lambda f2:",k,",l:",l);
       if k>0:
         return f2(k-1,l)
       return k;
    }
    r = f2(x,1);
    r2 =f2(x+1,2);
    print("  xyz:",x,y,z,",r=",r,",r2=",r2);
}
f1 =f
def Test(f0):
    print("call lambda")
    f0(100,2,3)
    print("end call")

Test(f1)
