VAR var1 = 1
VAR var2 = "hello"

{var2} line 1 {var1}
~ var1 = 5
<> {var2} line 2 {var1}
~ var2 = "test"
<> {var2} line 3 {var1}
