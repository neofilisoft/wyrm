ภาษา Wyrm หรือ wyrmlang (.wyr) Specification v2.2.0:

1. ประกาศฟังก์ชัน: `fn`
2. ค่าว่างเปล่า: `null`
3. Boolean: `true`/`false`
4. ตัวดำเนินการตรรกะ (Operators): `&&`, `||`, `!` (รองรับทั้ง `&&`, `||` และ `and`, `or`, `not` เป็น alias กัน)
5. การเปรียบเทียบ: `==`, `!=`, `<`, `>`, `<=`, `>=`
6. เงื่อนไข: `if`, `elif`, `else`
7. Block Style: `{ }`
8. ลูป: `do` / `til` (หรือ `repeat` / `til` เป็น alias)
9. Array: ใช้ `[1, 2, 3]` รองรับการเข้าถึงดัชนี (`arr[0]`) และแก้ไขค่าตามดัชนี (`arr[0] = 10`)
10. การนำเข้าโมดูล: `use module.wyr` หรือ `use module.wyr;`
11. Package Manager: `wyrpkg` (คำสั่ง `install`, `remove`, `list`)
12. Multi-arg Print: `print(a, b, c)` หรือ `print a, b, c` คั่นด้วยช่องว่าง
13. Comments: `//`, `/* */`, `///` (เหมือนภาษาตระกูล C)
14. Semicolon: ใส่หรือไม่ใส่ก็ได้
15. บล็อก unsafe: `unsafe { }` สำหรับเรียกใช้ raw memory
16. ระบบ Ownership และ RAII: จัดการหน่วยความจำอัตโนมัติตามขอบเขตของ Scope
17. Arena Allocator: จองเมมโมรี่เป็นภูมิภาคด้วยคำสั่ง `arena`

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
- `do { ... } til (cond)` (หรือใช้ `repeat` แทน `do` เป็น alias) - ต้องมีวงเล็บครอบเงื่อนไข C-style
  ```wyrm
  i = 0
  do {
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

### Ownership & RAII
- ตรวจสอบความเป็นเจ้าของของตัวแปรตั้งแต่ช่วง Compile-time ป้องกัน Use-After-Free
  ```wyrm
  owned x = [1, 2, 3]
  y = x  // สิทธิ์ความเป็นเจ้าของย้ายไปยัง y, x จะไม่สามารถเข้าถึงได้อีกต่อไป
  ```

### Arena Allocator
- เพิ่มความเร็วและควบคุม lifetime ของข้อมูลจำนวนมาก
  ```wyrm
  arena buf(256)        // ประกาศ arena ขนาด 256 ไบต์
  p = buf.alloc(64)     // จัดสรรหน่วยความจำภายใน arena
  // เมื่อจบบล็อกหรือฟังก์ชันนี้ arena จะถูกทำลายและคืนหน่วยความจำอัตโนมัติ (RAII)
  ```

### Unsafe Blocks & Raw Memory
- บังคับการเขียนโค้ดจัดการ Pointer หรือจัดการหน่วยความจำแบบแมนนวลให้ทำในบล็อก `unsafe` เท่านั้น
  ```wyrm
  unsafe {
      ptr = malloc(64)       // จัดสรร raw memory
      ptr = realloc(ptr, 128) // ปรับขนาดพื้นที่หน่วยความจำ
      free(ptr)              // คืนหน่วยความจำด้วยตัวเอง
  }
  // หากลืม free ตัวแปร ptr จะมีระบบ fallback ในระดับขอบเขต (Scope-based RAII) ช่วยทำลายหน่วยความจำที่คงค้างทั้งหมดให้อัตโนมัติเมื่อพ้นบล็อก
  ```