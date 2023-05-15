# WARNING: The code below is just a rough version of file formatter for this project!
#   In no circumstances this code should be considered a main part of the project or treated as such!
#   It belongs here for demonstrational purposes only!
#   It is recommended to use other text-to-wordlist formatters available.

file = open("assets/comedy_of_errors.txt", "r")

text = file.read()
for char in range(0, 256):
    if chr(char).isalpha(): continue
    text = text.replace(chr(char), " ")

text = text.lower()

words = text.split()

print("Read", len(words), "words.")
print("Max word length: ", max([len(word) for word in words]))
print("Sample words:", *words[:10])

file.close()


out = open("assets/sample.wordlist", "wb")
out.write(b"".join([bytearray((word + '\0' * (32 - len(word))).encode("utf-8")) for word in words]))
out.close()
