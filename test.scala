object Main extends App {
  def isPrime(n: Int): Int = {
    for (i <- 2 to n / 2 + 1) {
      if (n % i == 0) return 0
    }
    return 1
  }
  var numPrimes = 0
  for (i <- 2 to 100000) numPrimes += isPrime(i)
  println(numPrimes)
}