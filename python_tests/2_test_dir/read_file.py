filepath = input()
print(f"Accessing file {filepath}")
with open(filepath, "r") as f:
    print("Test file reads:", f.read())
