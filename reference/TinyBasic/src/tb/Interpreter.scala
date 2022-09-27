package tb

import tb.Parser._
import scala.collection.mutable.Buffer
import scala.collection.mutable.Stack
import scala.annotation.tailrec

object Interpreter:
  def run(sts: Seq[Statement], debug: Boolean): Unit =
    new Interpreter(sts, debug).run()

class Interpreter private (sts: Seq[Statement], debug: Boolean):
  val vars = Array.fill(26)(0.toShort)
  val array = Buffer[Short]()
  val loopStack = Stack[(Int, Jump)]()
  val lineNums =
    (for ((Statement(Some(Number(n)), c), i) <- sts.zipWithIndex) yield (n.toInt, i)).toMap
  val startingStatement =
    val mapps = for (Statement(_, RunCommand(ln)) <- sts) yield ln.map(_.v.toInt)
    mapps.headOption.flatten.map(lineNums.get).flatten.getOrElse(0)

  def resolveValue(v: Value): Short =
    v match {
      case Number(s) => s.toShort
      case RandomValue(low, high) =>
        val lo = resolveValue(low)
        val hi = resolveValue(high)
        util.Random.between(lo, hi).toShort

      case LogicalOp(l, '|', r) => if (resolveValue(l) != 0 || resolveValue(r) != 0) 1 else 0
      case LogicalOp(l, '&', r) => if (resolveValue(l) != 0 && resolveValue(r) != 0) 1 else 0
      case RelationalOp(l, '>', r) => if (resolveValue(l) > resolveValue(r)) 1 else 0
      case RelationalOp(l, '<', r) => if (resolveValue(l) < resolveValue(r)) 1 else 0
      case RelationalOp(l, '≥', r) => if (resolveValue(l) >= resolveValue(r)) 1 else 0
      case RelationalOp(l, '≤', r) => if (resolveValue(l) <= resolveValue(r)) 1 else 0
      case RelationalOp(l, '=', r) => if (resolveValue(l) == resolveValue(r)) 1 else 0
      case RelationalOp(l, '≠', r) => if (resolveValue(l) != resolveValue(r)) 1 else 0
      
      case AdditiveOp(l, '+', r) => (resolveValue(l) + resolveValue(r)).toShort
      case AdditiveOp(l, '-', r) => (resolveValue(l) - resolveValue(r)).toShort

      case MultiplicativeOp(l, '*', r) => (resolveValue(l) * resolveValue(r)).toShort
      case MultiplicativeOp(l, '/', r) => (resolveValue(l) / resolveValue(r)).toShort
      case MultiplicativeOp(l, '%', r) => (resolveValue(l) % resolveValue(r)).toShort

      case UnaryOp('-', u) => (-resolveValue(u)).toShort
      case UnaryOp('+', u) => (+resolveValue(u)).toShort

      case Variable(v) => vars(v - 'a')

      case ArrayReference(a) =>
        val offset = resolveValue(a)
        array.padToInPlace(offset + 1, 0.toShort)
        array(offset)

      case _ => sys.error(s"Unexpected relational op: $v")
    }
  def assignStr(mem: Memory, s: String): Unit =
    mem match {
      case ArrayReference(a) =>
        val v = resolveValue(a)
        array.padToInPlace(v + s.length, 0)
        for (i <- s.indices) array(v + i) = s.charAt(i).toShort
      case _ => sys.error(s"Assigning string \"$s\" to non-array $mem")
    }
  def assignMem(mem: Memory, value: Value): Unit =
    value match {
      case StrLiteral(s) => assignStr(mem, s)
      case _ => mem match {
        case Variable(v) =>
          vars(v - 'a') = resolveValue(value)
        case ArrayReference(a) =>
          val offset = resolveValue(a)
          array.padToInPlace(offset + 1, 0.toShort)
          array(offset) = resolveValue(value)
      }
    }
    
  def runFromEnd(i: Int, cs: Int = 0): Boolean =
    sts(i).command match {
      case IfCommand(_) | ForCommand(_,_) | UntilCommand(_) | WhileCommand(_) => return runFromEnd(i+1, cs+1)
      case EndCommand if cs > 0 => return runFromEnd(i+1, cs-1)
      case EndCommand => return runLn(i+1)
      case _ => return runFromEnd(i+1, cs)
    }
  def debugPrintln(x: Any) = if (debug) println(x)
  def printFormat(formatter: Seq[Char | Formatter], values: Seq[Value]): Unit = {
    if (formatter.isEmpty && !(values.isEmpty))
      sys.error(s"Extra values: ${values.mkString(", ")} for format string")
    else formatter.headOption.map {
      case StrFmt =>
        values.headOption match {
          case Some(m: ArrayReference) =>
            def printStr(m: ArrayReference): Unit = {
              val rm = resolveValue(m)
              if (rm != 0)
                print(rm.toChar)
                printStr(ArrayReference(AdditiveOp(m.to, '+', Number("1"))))
            }
            printStr(m)
            printFormat(formatter.tail, values.tail)
          case _ =>
            sys.error("Expected string-type value for formatter")
        }
      case CharFmt =>
        print(resolveValue(values.headOption.getOrElse(sys.error(s"Not enough values in formatter"))).toChar)
        printFormat(formatter.tail, values.tail)
      case SIntFmt =>
        print(resolveValue(values.headOption.getOrElse(sys.error(s"Not enough values in formatter"))))
        printFormat(formatter.tail, values.tail)
      case UIntFmt =>
        print(resolveValue(values.headOption.getOrElse(sys.error(s"Not enough values in formatter"))) & 0xff)
        printFormat(formatter.tail, values.tail)
      case c: Char =>
        print(c)
        printFormat(formatter.tail, values)
    }
  }
  def runLn(i: Int): Boolean =
    if (i >= sts.size)
      debugPrintln("Reached the end of program without a STOP command")
      return false
    else
      debugPrintln(sts(i))
      sts(i).command match {
        case LetCommand(mem, value) =>
          assignMem(mem, value)
          runLn(i+1)
        case PrintCommand(formatter, value) =>
          printFormat(formatter, value)
          runLn(i+1)
        case GotoCommand(value) =>
          // TODO: Fix callstack in this case
          val dest = resolveValue(value)
          if (lineNums.contains(dest))
            runLn(lineNums(dest))
          else
            sys.error(s"Attempt to goto non-existant line $dest")
        case CallCommand(value) =>
          val dest = resolveValue(value)
          if (lineNums.contains(dest))
            return runLn(lineNums(dest)) || runLn(i+1)
            
          else
            sys.error(s"Attempt to goto non-existant line $dest")
        case EndCommand =>
          def findN: Int = {
            var n = 0
            for (st <- i to 0 by -1) {
              if (sts(st).command == EndCommand) n += 1
              if (sts(st).command.isInstanceOf[Jump]) {
                n -= 1
                if (n <= 0) return st
              }
            }
            sys.error(s"Unmatched end at $i")
          }
          val fn = findN
          sts(fn).command.asInstanceOf[Jump] match {
            case ForCommand(m, RangeSpec(Number(f), Number(t), x, inclusive)) =>
              val s = x.map(resolveValue andThen (_.toInt)).getOrElse(1)
              if (inclusive && (f.toInt until t.toInt by s contains resolveValue(m)))
                assignMem(m, AdditiveOp(m, '+', Number(s"$s")))
                runLn(fn + 1)
              else if (!inclusive && (f.toInt until t.toInt - 1 by s contains resolveValue(m)))
                assignMem(m, AdditiveOp(m, '+', Number(s"$s")))
                runLn(fn + 1)
              else runLn(i + 1)
            case WhileCommand(pred) =>
              runLn(fn)
            case UntilCommand(pred) =>
              runLn(fn)
            case f: ForCommand =>
              sys.error(s"Unprocessed for command $f on loop stack")
            case _: IfCommand => runLn(i + 1)
          }
        case c @ IfCommand(pred) =>
          if (resolveValue(pred) == 0)
            return runFromEnd(i + 1)
          else return runLn(i + 1)
        case InputCommand(prompt, value) =>
          def input(): Unit = {
            print(prompt)
            val x = io.StdIn.readLine
            if (x == "") assignMem(value, Number("0"))
            else x.toShortOption.map(_ => assignMem(value, Number(x))).getOrElse {
              println("INVALID. TRY AGAIN.")
              input()
            }
          }
          input()
          runLn(i+1)
        case ListCommand(from, to) => ???
        case RunCommand(n) => ???
        case WhileCommand(pred) =>
          if (resolveValue(pred) != 0) runLn(i + 1)
          else runFromEnd(i + 1)
        case UntilCommand(pred) =>
          if (resolveValue(pred) == 0) return runLn(i + 1)
          else return runFromEnd(i + 1)
        case ForCommand(mem, range) =>
          val start = resolveValue(range.from)
          val end = resolveValue(range.to)
          val step = range.step.map(resolveValue).getOrElse {
            if (end >= start) 1.toShort else -1.toShort
          }
          assignMem(mem, range.from)
          if (range.inclusive) {
            if (start.toInt to end by step contains start) runLn(i + 1)
            else runFromEnd(i + 1)
          } else {
            if (start.toInt until end by step contains start) runLn(i + 1)
            else runFromEnd(i + 1)
          }
        case ReturnCommand => return true
        case StopCommand => return false
        case RemCommand => runLn(i+1)
      }

  def run(): Unit =
    debugPrintln(s"Starting execution from line $startingStatement")
    runLn(startingStatement)
    debugPrintln(s"Vars: ${('a' to 'z' zip vars).mkString(" ")}")
    debugPrintln(s"Mem: $array")
    
