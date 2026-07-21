# แผนพัฒนา Wyrm เป็นภาษาระบบ (System Programming Language)

## เป้าหมายหลัก (Design Goals)

| เป้าหมาย | รายละเอียด |
|----------|------------|
| ประสิทธิภาพ | ใกล้เคียง C/C++/Rust - ไม่มี GC, คอมไพล์เป็น native |
| ความปลอดภัยของ Memory | Rust > **Wyrm** > C++ |
| ความง่ายในการใช้ | ง่ายกว่า Rust (ไม่มี borrow checker ที่ซับซ้อน) แต่ปลอดภัยกว่า C++ |

---

## Memory Safety Model

ระบบ memory ของ Wyrm ถูกออกแบบให้อยู่ระหว่าง Rust กับ C++:

```
[ปลอดภัยสูงสุด]  Rust   -- Borrow Checker + Ownership + ไม่มี raw pointer ที่ไม่ปลอดภัย
                  Wyrm   -- Ownership + RAII + Safe Reference + Optional unsafe{}
[ปลอดภัยน้อยกว่า] C++   -- RAII + Smart Pointers + raw pointer (ยังอันตรายได้)
```

### กฎ Memory ของ Wyrm

1. **Ownership** - ทุก value มีเจ้าของหนึ่งคน (คล้าย Rust แต่ไม่มี lifetime annotations)
2. **RAII** - ทรัพยากรถูก free อัตโนมัติเมื่อออกจาก scope (เหมือน C++/Rust)
3. **ไม่มี raw pointer** ในโค้ดทั่วไป - ใช้ reference แทน
4. **Reference counting** สำหรับ shared ownership (`rc<T>`)
5. **`unsafe {}` block** สำหรับโค้ด low-level ที่จงใจข้ามกฎ (คล้าย Rust)
6. **ไม่มี Garbage Collector** - memory เป็น deterministic และ predictable

### ตัวอย่างไวยากรณ์ Memory ในอนาคต

```wyrm
// Ownership - ค่าเดียว เจ้าของเดียว
fn main() {
    var data = alloc(1024)   // allocate 1024 bytes บน heap
    use_data(data)           // data ถูก move ไป use_data
    // data ถูก free อัตโนมัติเมื่อออกจาก scope ของ use_data
}

// Reference - ยืมค่าโดยไม่ต้อง move ownership
fn print_len(ref arr: [i32]) {
    print("Length:", len(arr))
}   // arr ไม่ถูก free ที่นี่ - เป็นแค่ borrow

// Shared ownership ด้วย reference counting
fn shared_example() {
    var shared = rc([1, 2, 3])  // rc<[i32]>
    var copy_a = shared          // refcount = 2
    var copy_b = shared          // refcount = 3
    // free อัตโนมัติเมื่อ refcount = 0
}

// unsafe block สำหรับ low-level operations
fn raw_memory() {
    unsafe {
        var ptr: *u8 = alloc_raw(512)
        ptr[0] = 0xFF
        free_raw(ptr)
    }
}
```

---

## สถานะปัจจุบัน (สิ่งที่ได้ทำแล้ว - v1.0.0)

1. [x] ตัวแปร `var` (mutable) และ `dec` (immutable constant)
2. [x] ตัวดำเนินการตรรกศาสตร์: `&&`, `||`, `!` และ alias `and`, `or`, `not`
3. [x] ค่าว่างเปล่า: `null`
4. [x] โครงสร้างควบคุม: `if/elif/else`, `repeat/til`
5. [x] ฟังก์ชัน `fn` พร้อม recursion
6. [x] ระบบโมดูล `use module.wyr` หรือ `use module.wyr;`
7. [x] Package manager `wyrpkg`
8. [x] Arrays `[1, 2, 3]` พร้อม index read/write
9. [x] Multi-argument print: `print(a, b, c)` คั่นด้วยช่องว่าง
10. [x] Built-ins: `print`, `input`, `int`, `float`, `str`, `len`, `type`, `abs`, `min`, `max`, `round`, `pow`
11. [x] Optional semicolons
12. [x] C11 Native Bytecode VM runner (`wyrmc`)

---

## ขั้นตอนการพัฒนา

### ระยะสั้น - Language Features (ยังต้องทำ)
- [ ] `break` / `continue` ใน loop
- [ ] String formatting: `"Hello, {name}!"` หรือ `fmt()`
- [ ] Primitive types: `i32`, `i64`, `f32`, `f64`, `u8`, `bool`
- [ ] Type annotations (optional): `var x: i32 = 10`
- [ ] `for item in collection { }` loop
- [ ] Standard library modules: `std.io`, `std.math`, `std.str`

### ระยะกลาง - Native Compiler
- [ ] สร้าง IR (Intermediate Representation) จาก AST
- [ ] สร้าง Code Generator: AST -> LLVM IR หรือ C ต้นฉบับ
- [ ] ระบบประเภท (Type System) สมบูรณ์แบบ
- [ ] Stack vs Heap allocation
- [ ] Reference semantics (ยืม value ไม่ต้อง copy)
- [ ] Ownership transfer

### ระยะยาว - Self-hosting
- [ ] Ownership system + safe reference
- [ ] `rc<T>` สำหรับ shared ownership
- [ ] `unsafe {}` block
- [ ] เขียน Wyrm compiler ด้วยภาษา Wyrm เอง (bootstrap)
- [ ] Standard library ครบ: `std.io`, `std.net`, `std.mem`, `std.os`

---

## เป้าหมายประสิทธิภาพ (Performance Targets)

| ตัวชี้วัด | เป้าหมาย |
|---------|---------|
| Runtime speed | >=90% ของ C สำหรับงานทั่วไป |
| Memory usage | Deterministic - ไม่มี GC pauses |
| Latency | Predictable - เหมาะกับ real-time systems |
| Startup time | <1ms (native binary) |
| Binary size | เทียบเท่า C/Rust - ไม่มี runtime overhead |

---

## คำสั่ง wyrmc ที่วางแผนไว้

```bash
wyrmc --version          # แสดงเวอร์ชัน
wyrmc run input.wyr      # ทำงานผ่าน Python VM (ปัจจุบัน)
wyrmc build input.wyr    # คอมไพล์เป็น native binary (อนาคต)
wyrmc build --release input.wyr   # optimize สูงสุด
wyrmc test               # รันการทดสอบทั้งหมด
wyrmc check input.wyr    # ตรวจสอบ type/memory โดยไม่รัน
```

---

## เปรียบเทียบกับภาษาอื่น

| คุณสมบัติ | C | C++ | **Wyrm** | Rust |
|----------|---|-----|---------|------|
| GC | ไม่มี | ไม่มี | ไม่มี | ไม่มี |
| Raw pointer | มี | มี | unsafe{} เท่านั้น | unsafe{} เท่านั้น |
| RAII | ไม่มี | มี | มี | มี |
| Ownership | ไม่มี | บางส่วน | มี | มี (เข้มงวดกว่า) |
| Borrow Checker | ไม่มี | ไม่มี | ไม่มี | มี |
| Reference Counting | ไม่มี | shared_ptr | rc<T> | Rc<T>/Arc<T> |
| ความง่าย | ปานกลาง | ยาก | ง่าย | ยาก |
| Performance | สูงสุด | สูงมาก | สูงมาก (เป้าหมาย) | สูงมาก |

---

## สรุปงานล่าสุด

- 2026-07-21: v1.0.0 release - interpreter, VM, module system, multi-arg print, indexed assignment
- 2026-07-21: กำหนด Memory Model - ความปลอดภัย Rust > Wyrm > C++, ประสิทธิภาพเทียบ C/C++/Rust