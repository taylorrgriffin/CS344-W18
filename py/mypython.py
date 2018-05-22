########################################################
### Program Py
### CS 344
### 02/20/18
### Taylor Griffin
########################################################
# Import needed libraries
import random
import sys

# Generate first 10 characters for string1
string1 = ""
for i in range(10):
    string1+=chr(random.randint(97,122))
# Add newline character
string1+='\n'
# Assemble string2
string2 = ""
for i in range(10):
    string2+=chr(random.randint(97,122))
string2+='\n'
# Assemble string3
string3 = ""
for i in range(10):
    string3+=chr(random.randint(97,122))
string3+='\n'

# Open first file
f= open("file1","w+")
# Write string to file
f.write(string1)
# Close first file
f.close()
# Open and write to second file
f= open("file2","w+")
f.write(string2)
f.close()
# Open and write to third file
f= open("file3","w+")
f.write(string3)
f.close()

# Print the three strings
sys.stdout.write(string1)
sys.stdout.write(string2)
sys.stdout.write(string3)

# Generate two random numbers
num1 = random.randint(1,42)
num2 = random.randint(1,42)
product = num1*num2

# Print two numbers and product
sys.stdout.write(str(num1)+'\n')
sys.stdout.write(str(num2)+'\n')
sys.stdout.write(str(product)+'\n')
