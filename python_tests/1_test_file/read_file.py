input()
print("Accessing file ./test.txt")
with open("test.txt", "r") as f:
    print("Test file reads:", f.read())
