package tb
import util.parsing.combinator._

object Parser extends RegexParsers:
  type Elem = Char

  def ws = accept(' ').*

  def eol: Parser[_] = ws ~ ('\n' ||| ';')

  def stLine[T](p: Parser[T]): Parser[T] =
    (p | ws ~> p) <~ ws ~ eol.?

  def statements: Parser[Seq[Statement]] =
    (stLine('\n') ^^^ Statement(None, RemCommand) ||| stLine(statement)).+
  case class Statement(lineNum: Option[Number], command: Command) {
    override def toString =
      s"${lineNum.map(_.v).getOrElse("").padTo(6, ' ')}$command"
  }
  def statement: Parser[Statement] =
    (statement_number <~ ws).? ~ command ^^ {
      case a~b => Statement(a,b)
    }
  def statement_number: Parser[Number] = number
  
  sealed trait Value
  def value: Parser[Value] = relational_op

  sealed trait Jump

  sealed abstract class BinOp(lhs: Value, op: Char, rhs: Value) extends Value {
    override def toString = s"($lhs $op $rhs)"
  }

  def opsP(v2: Parser[Value], ops: Seq[Char], fn: (Value, Char, Value) => Value): Parser[Value] =
    chainl1(v2, ws ~> (acceptMatch("Operator", {
      case c if ops contains c => (x: Value, y: Value) => fn(x, c, y)
    })) <~ ws)

  case class RelationalOp(v1: Value, op: Char, v2: Value) extends BinOp(v1, op, v2)
  def relational_op: Parser[Value] =
    opsP(additive_op, Seq('>', '<', '≥', '≤', '=', '≠'), RelationalOp.apply)

  case class AdditiveOp(v1: Value, op: Char, v2: Value) extends BinOp(v1, op, v2)
  def additive_op: Parser[Value] =
    opsP(multiplicative_op, Seq('+', '-'), AdditiveOp.apply)

  case class MultiplicativeOp(v1: Value, op: Char, v2: Value) extends BinOp(v1, op, v2)
  def multiplicative_op: Parser[Value] =
    opsP(unary_op, Seq('*', '/', '%'), MultiplicativeOp.apply)
  case class UnaryOp(op: Char, on: Value) extends Value
  def unary_op: Parser[Value] = (
    ('-' <~ ws) ~ unary_op |||
    ('+' <~ ws) ~ unary_op
  ) ^^ (x => UnaryOp(x._1, x._2)) |||
    memory |||
    number |||
    strLiteral |||
    random |||
    '(' ~ ws ~> value <~ ws ~ ')'

  sealed trait Memory extends Value {
    def prompt: String = s"> "
  }
  def memory =
    variable |||
    array_reference

  case class Variable(name: Char) extends Memory {
    override def toString = s"$name"
  }
  def variable = ((
    acceptIf('A' to 'Z' contains _)(c => s"Expected a variable name, got $c") |||
    acceptIf('a' to 'z' contains _)(c => s"Expected a variable name, got $c")
  ) ^^ (_.toLower)).+ ^^ (_.head) ^^ Variable.apply

  case class ArrayReference(to: Value) extends Memory {
    override def toString = s"[$to]"
  }
  def array_reference =
    ('[' ~ ws ~> value <~ ws ~ ']') ^^ ArrayReference.apply
  def strLiteral = '"' ~> (acceptIf(_ != '\"')(x => s"Expected a non-quote, got: $x").* ^^ (StrLiteral apply _.mkString)) <~ '"'
  case class StrLiteral(s: String) extends Value {
    override def toString = s"\"$s\""
  }
  case class Number(v: String) extends Value {
    override def toString = v
  }
  def number: Parser[Number] = acceptIf(_.isDigit)(x => s"Expected a digit, got $x").+ ^^ (_.mkString) ^? {
    case x if x.toShortOption.isDefined => Number(x)
  } ||| accept('\'') ~> acceptIf(_ != '\'')(x => s"Expected a non-quote, got: $x") <~ '\'' ^^ (x => Number(s"${x.shortValue}"))
  sealed trait Command
  def command: Parser[Command] =
    if_command     |||
    while_command  |||
    until_command  |||
    for_command    |||
    input_command  |||
    list_command   |||
    run_command    |||
    let_command    |||
    print_command  |||
    goto_command   |||
    call_command   |||
    return_command |||
    stop_command   |||
    end_command    |||
    rem_command

  case class RandomValue(low: Value, high: Value) extends Value
  def random =
    cmd("rnd") ~ ws ~ '(' ~ ws ~> (value <~ ws ~ "," ~ ws).? ~ value <~ ws ~ ')' ^^ {
      case a~b => RandomValue(a.getOrElse(Number("0")),b)
    }

  def cmd(s: String): Parser[?] =
    val p =  s.toSeq.map((c: Char) => accept(c.toLower) | accept(c.toUpper)).foldLeft[Parser[?]](success(()))(_ ~ _)
    (p | (ws ~> p)) <~ ws

  def notNewline: Parser[String] =
    acceptIf(_ != '\n')(c => s"Expected non-newline, found $c").* ^^ (_.mkString)

  case class LetCommand(memory: Memory, value: Value) extends Command {
    override def toString = s"LET $memory = $value"
  }
  def let_assign = memory ~ (ws ~ '=' ~ ws ~> value) ^^ {
      case a ~ b => LetCommand(a, b)
    }
  def let_command =
    cmd("let ") ~ ws ~> let_assign

  sealed trait Formatter
  case object CharFmt extends Formatter {
    override def toString = "~c"
  }
  case object SIntFmt extends Formatter{
    override def toString = "~i"
  }
  case object UIntFmt extends Formatter {
    override def toString = "~u"
  }
  case object StrFmt extends Formatter {
    override def toString = "~s"
  }
  case class PrintCommand(formatter: Seq[Char | Formatter], args: Seq[Value]) extends Command {
    override def toString = s"PRINT \"${formatter.map {
      case '~' => "~~"
      case '\n' => "~n"
      case x => x
    }.mkString}\" ${args.mkString(" ")}"
  }
  val cFmt = 'c' ^^^ CharFmt
  val iFmt = 'i' ^^^ SIntFmt
  val uFmt = 'u' ^^^ UIntFmt
  val sFmt = 's' ^^^ StrFmt
  val nFmt = 'n' ^^^ '\n'
  val tFmt = '~' ^^^ '~'
  val qFmt = '"' ^^^ '"'
  def format_char: Parser[Char | Formatter] =
    (cFmt ||| sFmt ||| iFmt ||| uFmt) ||| (nFmt ||| tFmt ||| qFmt)
  def format_string =
    ('~' ~> format_char) |||
    acceptIf(x => x != '"' && x != '\n')(x => s"Expected a format string char, got $x")
  def print_command = (cmd("print ") ~ ws ~ '"' ~> format_string.* <~ '"') ~ (ws ~> repsep(value, accept(' ').+)) ^^ {
    case x~y => PrintCommand(x, y)
  }

  case class GotoCommand(value: Value) extends Command {
    override def toString = s"GOTO $value"
  }
  def goto_command = cmd("goto ") ~ ws ~> value ^^ GotoCommand.apply

  case class CallCommand(value: Value) extends Command {
    override def toString = s"CALL $value"
  }
  def call_command = cmd("call ") ~ ws ~> value ^^ CallCommand.apply

  case object ReturnCommand extends Command {
    override def toString = s"RETURN"
  }
  def return_command = cmd("return") ^^^ ReturnCommand

  case object EndCommand extends Command {
    override def toString = s"END"
  }
  def end_command = stLine(cmd("end")) ^^^ EndCommand

  case class IfCommand(predicate: Value) extends Command with Jump {
    override def toString = s"IF $predicate"
  }
  def if_command = stLine(cmd("if ") ~> value) ^^ {
    case a => IfCommand(a)
  }

  case class InputCommand(prompt: String, dest: Memory) extends Command {
    override def toString = s"INPUT \"$prompt\" $dest"
  }
  def stringContent = (acceptIf(x => x != '\n' && x != '"')(x => s"Expected a string char, found $x")) ||| (accept('\\') ~ accept('\"') ^^^ "\"")
  def string = '"' ~> stringContent.+ <~ '"' ^^ (_.mkString)
  def input_command = cmd("input ") ~> (ws ~> string.? <~ ws) ~ memory ^^ {
    case Some(a) ~ b => InputCommand(a, b)
    case None ~ b => InputCommand(b.prompt, b)
  }

  case class ListCommand(from: Value, to: Value) extends Command {
    override def toString = s"LIST $from $to"
  }
  def list_command =
    cmd("list") ^^^ ListCommand(Number("0"), Number("32767")) |
    cmd("list ") ~> value ^^ (v => ListCommand(v, v))         |
    cmd("list ") ~> value ~ (ws ~> value) ^^ {case a~b => ListCommand(a, b)}

  def block = statements <~ cmd("end")

  // TODO: provide input
  case class RunCommand(start: Option[Number]) extends Command {
    override def toString = if (start.isDefined) s"RUN ${start.get.v}" else "RUN"
  }
  def run_command =
    cmd("run") ^^^ RunCommand(None) |
    cmd("run ") ~> number ^^ Some.apply ^^ RunCommand.apply

  case class WhileCommand(predicate: Value) extends Command with Jump  {
    override def toString = s"WHILE $predicate"
  }
  def while_command = stLine(cmd("while ") ~> value) ^^ {
    case a => WhileCommand(a)
  }    

  case class UntilCommand(predicate: Value) extends Command with Jump  {
    override def toString = s"UNTIL $predicate"
  }
  def until_command = stLine(cmd("until ") ~> value)^^ {
    case a => UntilCommand(a)
  }

  case class RangeSpec(from: Value, to: Value, step: Option[Value], inclusive: Boolean)
  def rangeSpec = value ~ (cmd(" to ") ^^^ true ||| cmd(" until ") ^^^ false) ~ value ~ (cmd(" by ") ~> value).? ^^ {
    case a ~ b ~ c ~ d => RangeSpec(a, c, d, b)
  }
  case class ForCommand(memory: Memory, range: RangeSpec) extends Command with Jump  {
    override def toString =
      if (range.step.isDefined)
        s"FOR $memory IN ${range.from} TO ${range.to} BY ${range.step.get}"
      else
        s"FOR $memory IN ${range.from} TO ${range.to}"
  }
  def for_command = stLine((cmd("for ") ~> memory <~ cmd(" in ")) ~ rangeSpec) ^^ {
    case a ~ b => ForCommand(a, b)
  }

  case object StopCommand extends Command {
    override def toString = "STOP"
  }
  def stop_command = cmd("stop") ^^^ StopCommand
    
  case object RemCommand extends Command {
    override def toString = "REM"
  }
  def rem_command = cmd("rem ") ~ notNewline ^^^ RemCommand
