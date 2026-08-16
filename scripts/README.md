# 📁 LOVEMACHINE CS:S - SCRIPTS, MODELS & CONFIGS

Thư mục này chứa toàn bộ các **Configs (.ini / profile)** và **Custom 3D Player Models (.mdl / .vtf / .vmt)** để bạn tùy biến game.

---

## 📂 Cấu trúc thư mục:

```
scripts/
├── configs/          # Nơi lưu trữ các file cấu hình cheat (.ini / profile)
│   ├── neverlose_profile.ini
│   └── global_profile.ini
│
└── models/           # Nơi chứa các gói Custom 3D Model nhân vật
    └── cissia_zzz/   # Model Cissia (Zenless Zone Zero)
        ├── models/   # Chứa file .mdl, .vtx, .vvd
        └── materials/# Chứa textures, shaders, .vmt, .vtf
```

---

## 🎮 Cách cài Custom 3D Model vào CS:S để hiển thị trong game:

Trong động cơ **Source Engine (CS:S)**, để game có thể render được bất kỳ Custom Model 3D nào (như Cissia ZZZ hay bất kỳ nhân vật anime nào bạn tải trên GameBanana):

1. Mở thư mục cài đặt game CS:S của bạn (ví dụ: `Counter-Strike Source/cstrike/`).
2. Copy 2 thư mục `models/` và `materials/` từ trong `scripts/models/cissia_zzz/` vào thư mục `cstrike/`:
   * `cstrike/models/...`
   * `cstrike/materials/...`
3. Mở menu cheat -> Vào tab **Visuals -> Player Info -> Custom 3D Player Model**:
   * Bật **"Add / Apply Custom 3D Model"**
   * Chọn model **"Cissia ZZZ (Zenless Zone Zero)"**
   * Bật **"Local Player Only"** (nếu chỉ muốn biến hình cho riêng bạn ở góc nhìn thứ 3 - Thirdperson).

---

## 🎨 Tùy chọn Model tích hợp sẵn (Không cần copy file):
* `Phoenix Terrorist (T)`
* `Leet Krew (T)`
* `SAS Gasmask (CT)`
* `GIGN SWAT (CT)`
* `Hostage (Scientist)`
