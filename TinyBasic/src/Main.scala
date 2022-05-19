
import util.parsing.input.CharSequenceReader
import tb.Interpreter

@main def main(files: String*) = 
  val input = io.Source.fromFile(files.headOption.getOrElse("sample/z.basic")).mkString
  val csr = CharSequenceReader(input)
  val res = tb.Parser.parseAll(tb.Parser.statements, csr)
  //println(res.map(_.mkString("\n", "\n", "")))
  res.map(Interpreter.run(_, debug = false)).getOrElse(println(res))
