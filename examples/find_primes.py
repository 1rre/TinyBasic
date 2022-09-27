
def isPrime(n):
  for i in range(2, n//2+1):
    if (not(n%i)):
      return 0
  return 1

numPrimes = 0

for i in range(2, 100001):
  numPrimes += isPrime(i)

print(str(numPrimes))