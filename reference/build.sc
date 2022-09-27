import mill._, scalalib._

object TinyBasic extends ScalaModule {
  def scalaVersion = "3.1.2"
  def ivyDeps = Agg(ivy"org.scala-lang.modules::scala-parser-combinators::2.1.1")
}