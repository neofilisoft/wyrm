ภาษา Wyrm หรือ wyrmlang (.wyr) Specification v1.2.0:

1. ประกาศฟังก์ชัน: `fn`
2. ค่าว่างเปล่า: `null`
3. Boolean: `true`/`false`
4. ตัวดำเนินการตรรกะ (Operators): `&&`, `||`, `!` (รองรับทั้ง `&&`, `||` และ `and`, `or`, `not` เป็น alias กัน)
5. การเปรียบเทียบ: `==`, `!=`, `<`, `>`, `<=`, `>=`
6. เงื่อนไข: `if`, `elif`, `else`
7. Block Style: `{ }`
8. ลูป: `repeat` / `til`
9. Array: ใช้ `[1, 2, 3]` รองรับการเข้าถึงดัชนี (`arr[0]`) และแก้ไขค่าตามดัชนี (`arr[0] = 10`)
10. การนำเข้าโมดูล: `use module.wyr` หรือ `use module.wyr;`
11. Package Manager: `wyrpkg` (คำสั่ง `install`, `remove`, `list`)
12. Multi-arg Print: `print(a, b, c)` หรือ `print a, b, c` คั่นด้วยช่องว่าง
13. Comments: `//`, `/* */`, `///` (เหมือนภาษาตระกูล C)
14. Semicolon: ใส่หรือไม่ใส่ก็ได้

---

## ไวยากรณ์ตัวอย่าง

### Block Style & Conditions
- ไม่มีคำว่า then หรือ do คั่นระหว่าง condition กับ `{`
  ```wyrm
  if x > 1 {
      print("greater")
  } elif x == 1 {
      print("equal")
  } else {
      print("less")
  }
  ```

### Loop Syntax
- `repeat { ... } til (cond)` - ต้องมีวงเล็บครอบเงื่อนไข C-style
  ```wyrm
  i = 0
  repeat {
      print(i)
      i = i + 1
  } til (i >= 5)
  ```

### Function & Module System
  ```wyrm
  use helper.wyr;

  fn add(a, b) {
      return a + b
  }

  fn main() {
      name = input("Enter name: ")
      print("Hello,", name)
      sum = add(5, 10)
      print("Sum is:", sum)
  }
  ```

### Arrays
  ```wyrm
  numbers = [10, 20, 30]
  numbers[0] = 99
  print("First element:", numbers[0])
  print("Array length:", len(numbers))
  ```