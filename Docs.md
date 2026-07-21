ภาษา Wyrm หรือ wyrmlang (.wyr):
ประกาศฟังก์ชัน: fn
ค่าว่างเปล่า: null
Boolean: true/false
ตัวดำเนินการตรรกะ (Operators): &&, ||, !  (รองรับทั้ง &&, || และ and, or เป็น alias กัน มือใหม่ใช้ and กับ or เพื่อเข้าใจง่าย แต่หากต้องการเขียนเร็วและกระชับใช้ && กับ ||)
ไม่เท่ากับ: !=
เงื่อนไข: elif
Block Style: { }
ลูป: repeat/til
String & Array: #
Comments: //, /* */, /// (เหมือนภาษาตระกูล C)
Notable Operators: ไม่มี
Semicolon: ใส่หรือไม่ใส่ก็ได้

Quiz: if/while/for ระหว่าง condition กับ { มีคำว่า then/do คั่นมั้ยหรือไม่มี
Answer: ไม่มี — { เปิด block เลย เช่น if x > 1 { ... }

Q: repeat { ... } til cond — cond ต้องมีวงเล็บครอบมั้ย
A: มีวงเล็บ — til (cond) แบบ C-style
เก็บ // ไว้เป็น comment ตามสเปค แล้วใช้ / ธรรมดาสำหรับ division (ตัด floor division ออก ใช้ int()/math.floor แทน)

// ตัวอย่าง Hello World แบบ Standard
fn main() {
    message = "Hello, World!"
    print(message)
}

// แบบไม่มีตัวแปร
fn main() {
    print("Hello, World!")
}

// แบบ Advanced
fn main() {
    // ส่งค่า "Hello, World!" เข้าไปที่พารามิเตอร์ชื่อ message ของฟังก์ชัน print
    print(message = "Hello, World!") 
}

// การนำเข้าโมดูล (ต้องพัฒนาในอนาคต)
// ใช้คำสั่ง 'use' คล้ายกับ import ในภาษาอื่นๆ
// ตัวอย่างการใช้งาน (ยังไม่ได้ implement):
// use std::io;
// use math::*;
// จากนั้นสามารถใช้ฟังก์ชันจากโมดูลนั้นได้โดยตรง