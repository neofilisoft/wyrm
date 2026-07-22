# คู่มือเขียนโปรแกรมภาษา Wyrm

Wyrm (`.wyr`) เป็นภาษาสคริปต์ที่ได้แรงบันดาลใจจากภาษาตระกูล C ปัจจุบันทำงานแบบ interpreter
(tree-walking และมี bytecode/VM อีกทาง) และมีแผนพัฒนาต่อไปเป็นภาษาระบบ (systems language)
เอกสารนี้อธิบายภาษาตาม **ที่ implement จริง** ในซอร์สโค้ด (`lexer.py`, `parser.py`, `ast.py`,
`interpreter.py` spec v1.2)

---

## 1. วิธีรันโค้ด Wyrm

```bash
# Python interpreter (tree-walking)
python -m wyrm path/to/file.wyr

# REPL
python -m wyrm

# Native C11 VM runner (bytecode path)
./wyrmc.exe path/to/file.wyr

# In the browser (online compiler, GitHub Pages build)
# just paste code into the editor and click Run
```

ถ้าไฟล์มีฟังก์ชัน `main()` ตัวอินเทอร์พรีเตอร์จะเรียก `main()` ให้อัตโนมัติหนึ่งครั้ง
หลังรันคำสั่งระดับบนสุด (top-level) จนจบ ไม่จำเป็นต้องเรียกเองก็ได้ (ดูข้อ 9)

---

## 2. คอมเมนต์ และเครื่องหมายจบคำสั่ง

```wyrm
// single-line comment
/* multi-line
   comment */
/// doc-style comment (still just a comment)

x = 1      // no semicolon needed
y = 2;     // semicolon optional, purely cosmetic
```

รองรับคอมเมนต์ทั้ง 3 แบบตามสไตล์ C เครื่องหมาย semicolon (`;`) ท้ายคำสั่ง
**ใส่หรือไม่ใส่ก็ได้เสมอ** ไม่มีผลต่อการทำงาน

---

## 3. ตัวแปรและการประกาศตัวแปร

Wyrm มี 3 วิธีผูกชื่อกับค่า

| รูปแบบ | ความหมาย | เปลี่ยนค่าใหม่ได้ไหม |
|---|---|---|
| `name = expr` | assignment แบบธรรมดา (dynamic) จะสร้างตัวแปรใน scope ปัจจุบันถ้ายังไม่มี | ได้ |
| `var name = expr` | ประกาศตัวแปรที่เปลี่ยนค่าได้ (mutable) แบบชัดเจน | ได้ |
| `dec name = expr` | ประกาศค่าคงที่ | **ไม่ได้** เขียนทับจะเกิด runtime error |

```wyrm
var age = 25
dec PI = 3.14159
count = 0          // plain assignment also works, no keyword required

count = count + 1  // OK
PI = 3.0           // RuntimeError: Cannot assign to constant 'PI'
```

ตัวดำเนินการ assignment แบบผสมก็รองรับ: `+=`, `-=`, `*=`, `/=`, `%=`
(ใช้กับสมาชิกอาร์เรย์ได้ด้วย เช่น `arr[i] += 1`)

`var`/`dec` เป็นทางเลือกด้านสไตล์ การเขียน `x = expr` เฉยๆ ครั้งแรกในสโคปนั้นจะทำงานเหมือน
ประกาศแบบ `var` โดยปริยาย ให้ใช้ `dec` เมื่อค่าต้องไม่ถูกเปลี่ยนแปลงอีกเลย

---

## 4. ชนิดข้อมูล

| ชนิด | ตัวอย่าง | หมายเหตุ |
|---|---|---|
| int | `42` | |
| float | `3.14` | |
| str | `"hello"` | รองรับ escape sequence เช่น `\n`, `\t`, `\"`, `\\` |
| bool | `true` / `false` | |
| null | `null` | ค่า "ไม่มีค่า" ของ Wyrm |
| list (array) | `[1, 2, 3]` | เก็บข้อมูลต่างชนิดกันในลิสต์เดียวได้ |

ฟังก์ชันในตัวสำหรับแปลงชนิด / ตรวจชนิดข้อมูล: `int(x)`, `float(x)`, `str(x)`, `type(x)`

```wyrm
print(type(42))       // "int"
print(type(3.14))     // "float"
print(type("hi"))     // "str"
print(type([1,2]))    // "list"
print(type(null))     // "null"
```

Wyrm **ไม่แปลงชนิดข้อมูลให้อัตโนมัติ** ระหว่างตัวเลขกับสตริง เช่น `"x = " + 5`
จะ error ต้องเขียน `"x = " + str(5)` แทน ส่วนตัวดำเนินการทางคณิตศาสตร์ (`+ - * / // %`)
มีพฤติกรรมคล้าย Python (`/` คือหารแบบได้ทศนิยม, `//` คือหารปัดลง)

---

## 5. ตัวดำเนินการ

```wyrm
// Arithmetic
+  -  *  /  //  %

// Comparison
==  !=  <  >  <=  >=

// Logical (symbol form and word form are interchangeable aliases)
&&  ||  !
and or  not

// Assignment
=  +=  -=  *=  /=  %=
```

`&&` กับ `and`, `||` กับ `or`, `!` กับ `not` เป็นคำที่แทนกันได้เป๊ะๆ (alias)
เลือกใช้แบบไหนก็ได้ตามความอ่านง่าย

---

## 6. สตริง และการพิมพ์ผลลัพธ์

```wyrm
name = "Aran"
print("Hello, " + name + "!")       // string concatenation with +
print("Hello,", name)               // multi-arg print, space-separated
print name, "is", 25, "years old"   // print also works without parentheses

// strings support indexing, like a read-only array of characters
first_char = name[0]
print(len(name))                    // string length via len()
```

`print` เขียนได้ทั้ง `print(a, b, c)` และแบบไม่มีวงเล็บ `print a, b, c` ถูกต้องทั้งคู่
และเมื่อมีหลาย argument จะถูกคั่นด้วยช่องว่างหนึ่งช่อง เหมือน `print()` ในภาษาสคริปต์ทั่วไป

---

## 7. เงื่อนไข

```wyrm
if score >= 90 {
    grade = "A"
} elif score >= 80 {
    grade = "B"
} else {
    grade = "F"
}
```

ไม่มีคำว่า `then`/`do` คั่นระหว่างเงื่อนไขกับ `{` ส่วน `elif`/`else` จะมีหรือไม่มีก็ได้
และต่อกันเป็นลูกโซ่ได้หลายอัน

---

## 8. ลูป

ตอนนี้ Wyrm มีลูปแบบเดียวคือ `repeat { ... } til (เงื่อนไข)` ซึ่งเป็นลูปแบบ post-condition
(เหมือน `do { } while(!cond)` ใน C) จะรัน body **อย่างน้อย 1 รอบเสมอ**

```wyrm
i = 0
repeat {
    print(i)
    i = i + 1
} til (i >= 5)          // parentheses around the condition are required
```

`break` ออกจากลูปทันที ส่วน `continue` จะข้ามไปตรวจเงื่อนไข `til` ของรอบถัดไป

```wyrm
i = 0
repeat {
    i = i + 1
    if i == 3 { continue }   // skip printing 3
    if i >= 5 { break }
    print(i)
} til (i >= 10)
```

ยังไม่มีคีย์เวิร์ด `while`/`for` ทุกลูปต้องเขียนเป็น `repeat/til` ถ้าต้องการรูปแบบ
"อาจไม่รันเลยก็ได้" ให้ครอบ body ด้วย `if` หรือวางเงื่อนไขออกจากลูปไว้ต้นสุดของ body แล้วใช้ `break`

---

## 9. ฟังก์ชัน

```wyrm
fn add(a, b) {
    return a + b
}

fn factorial(n) {
    if n <= 1 {
        return 1
    } else {
        return n * factorial(n - 1)   // recursion works
    }
}

fn main() {
    print("5 + 3 =", add(5, 3))
    print("5! =", factorial(5))
}
```

ฟังก์ชันเป็น first-class value และรองรับ closure ตามหลัก lexical scoping
คือฟังก์ชันจะจดจำ environment ตอนที่ถูกประกาศไว้ ทำให้ฟังก์ชันที่ซ้อนอยู่ข้างในสามารถอ่าน
(และเขียนทับผ่านกลไก assignment) ตัวแปรจาก scope ภายนอกได้ ถ้าเรียกฟังก์ชันแล้วส่ง argument
ไม่ครบ พารามิเตอร์ที่ขาดจะได้ค่า `null` แทนที่จะ error และถ้าฟังก์ชันไม่มี `return`
จะคืนค่า `null` โดยปริยาย ฟังก์ชัน `main()` ถ้ามีอยู่ในไฟล์ จะถูกเรียกอัตโนมัติหนึ่งครั้งหลังจาก
statement ระดับบนสุดรันจบ ปกติไม่ต้องเรียกเองอีก (ต่างจากไฟล์ตัวอย่างเก่าบางไฟล์ในโปรเจกต์
ที่ยังเรียก `main()` ซ้ำอีกครั้งด้วยความเคยชิน)

---

## 10. อาร์เรย์

```wyrm
numbers = [1, 2, 3, 4, 5]
numbers[0] = 10          // index assignment
numbers[0] += 5           // compound assignment on an element

mixed = [1, "hello", 3.14, true]   // arrays can hold mixed types

append(numbers, 99)       // add to the end
last = pop(numbers)       // remove & return the last element

print("length:", len(numbers))
```

อาร์เรย์นับ index เริ่มจาก 0 `append`/`pop` จะแก้ไขอาร์เรย์เดิมโดยตรง (mutate in place)
ปัจจุบันยังไม่รองรับการ slice (`arr[1:3]`) และไม่รองรับ index ติดลบ

---

## 11. ฟังก์ชันในตัว

| ฟังก์ชัน | หน้าที่ |
|---|---|
| `print(...)` | พิมพ์ค่า คั่นด้วยช่องว่าง |
| `input(prompt)` | อ่านค่าจากผู้ใช้ |
| `int(x)` / `float(x)` / `str(x)` | แปลงชนิดข้อมูล |
| `len(x)` | ความยาวของสตริงหรืออาร์เรย์ |
| `type(x)` | คืนชื่อชนิดข้อมูลเป็นสตริง |
| `abs(x)` | ค่าสัมบูรณ์ |
| `min(...)` / `max(...)` | ค่าต่ำสุด-สูงสุด |
| `round(x)` | ปัดเศษ |
| `pow(x, y)` | ยกกำลัง |
| `append(list, value)` | เพิ่มค่าท้ายลิสต์ |
| `pop(list)` | ลบและคืนค่าตัวสุดท้าย |

---

## 12. โมดูล

```wyrm
use math_helpers.wyr;   // semicolon optional
use math_helpers.wyr

fn main() {
    print(add_nums(1, 2))   // functions/consts defined in math_helpers.wyr are now visible
}
```

`use` จะโหลดและรันไฟล์ `.wyr` อื่น (ค้นหาไฟล์แบบสัมพัทธ์กับตำแหน่งไฟล์ปัจจุบัน)
แล้วดึงฟังก์ชัน/ตัวแปรระดับบนสุดของไฟล์นั้นเข้ามาอยู่ใน scope เดียวกัน คล้าย `#include`
แบบง่ายๆ ยังไม่มีระบบ namespace หรือ alias ทุกอย่างจะไปรวมอยู่ใน global scope เดียวกันหมด

---

## 13. ข้อผิดพลาด

ข้อผิดพลาดที่พบบ่อยตอนพัฒนาโปรแกรม

| สถานการณ์ | Error |
|---|---|
| ใช้ตัวแปรที่ยังไม่ประกาศ | `Undefined variable: 'x'` |
| เขียนทับค่าคงที่ `dec` | `Cannot assign to constant 'PI'` |
| หารด้วยศูนย์ | `Division by zero` |
| บวกต่างชนิดกัน (เช่น `str + int`) | `TypeError` ระดับ Python ให้ครอบด้วย `str()` |

---

## 14. ตัวอย่างโปรแกรมเต็ม

```wyrm
// average.wyr - averages an array of scores and prints the grade

dec PASS_MARK = 50

fn average(scores) {
    total = 0
    i = 0
    repeat {
        total = total + scores[i]
        i = i + 1
    } til (i >= len(scores))
    return total / len(scores)
}

fn main() {
    scores = [85, 92, 78, 96, 60]
    avg = average(scores)

    print("Average score:", avg)

    if avg >= PASS_MARK {
        print("Result: PASS")
    } else {
        print("Result: FAIL")
    }
}
```

---

## 15. สิ่งที่ยังไม่มีในภาษา

ตามแผนพัฒนาของโปรเจกต์ (`TODO_SYSTEM_LANGUAGE.md`) สิ่งที่ยังไม่ implement ได้แก่
ลูป `while`/`for`, struct/class, การ slice หรือ index ติดลบของอาร์เรย์, ฟังก์ชันช่วยจัดรูปแบบ
สตริง, standard library ที่มากกว่า built-in ด้านบน และฟีเจอร์ระบบอย่าง ownership/`unsafe {}`
ที่วางแผนไว้ ก่อนจะพึ่งพาฟีเจอร์ใดที่ไม่ได้กล่าวถึงในเอกสารนี้ ให้ตรวจสอบ
`TODO_SYSTEM_LANGUAGE.md` และ `implementation_plan.md` ในโปรเจกต์เพื่อดูสถานะล่าสุดก่อน
