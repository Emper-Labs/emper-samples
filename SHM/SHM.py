# file này để minh họa dao động điều hòa đơn giản, với 3 phần:
# 1. Đồ thị x(t)
# 2. Con lắc lò xo
# 3. Chuyển động tròn đều và hình chiếu lên trục Ox
# sẽ được thêm vào emper trong tương lai

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.patches import Arc

# =========================
# Tham số
# =========================
A     = 4
omega = 2 * np.pi
phi   = -2 * np.pi / 3
T     = 2 * np.pi / omega
t_max = 2 * T

# Hệ số đệm quanh biên độ
PAD = 0.25          # 25% A ở mỗi bên
XMIN_CIRC = -(A + PAD * A)
XMAX_CIRC =  (A + PAD * A)

# =========================
# Figure
# =========================
fig = plt.figure(figsize=(13, 10))

gs = fig.add_gridspec(
    3, 1,
    left=0.08, right=0.72, top=0.96, bottom=0.06,
    hspace=0.5
)

ax1 = fig.add_subplot(gs[0])
ax2 = fig.add_subplot(gs[1])
ax3 = fig.add_subplot(gs[2])

# =========================
# 1. Đồ thị x(t)
# =========================
ax1.set_xlim(0, t_max)
ax1.set_ylim(-A * 1.25, A * 1.25)               # <-- theo A
ax1.set_xlabel("t (s)")
ax1.set_ylabel("x (cm)")
ax1.axhline(0, color="k", linewidth=0.5)
ax1.grid(True, alpha=0.3)
ax1.set_title(f"Đồ thị li độ x = A·cos(ωt + φ)   với A = {A}")

t_arr = np.linspace(0, t_max, 2000)
x_arr = A * np.cos(omega * t_arr + phi)
ax1.plot(t_arr, x_arr, "b", linewidth=1.5)
ax1.axhline( A, color="gray", linestyle=":", linewidth=0.7)
ax1.axhline(-A, color="gray", linestyle=":", linewidth=0.7)
ax1.text(t_max * 0.01,  A, " +A", va="bottom", fontsize=9)
ax1.text(t_max * 0.01, -A, " -A", va="top",    fontsize=9)
point_xt, = ax1.plot([], [], "ro", markersize=7)

# =========================
# 2. Con lắc lò xo
# =========================
WALL_X = -(A + 0.6)                             # tường nằm ngoài -A
ax2.set_xlim(WALL_X - 0.3, A + 0.6)
ax2.set_ylim(-0.6, 0.6)
ax2.set_xlabel("x (cm)")
ax2.set_yticks([])
ax2.axhline(0, color="k", linewidth=0.5)
ax2.set_title("Dao động của vật")

for xv in (-A, A):
    ax2.axvline(xv, color="gray", linestyle=":", linewidth=0.7)

particle,    = ax2.plot([], [], "o", markersize=15, color="red", zorder=5)
spring_line, = ax2.plot([], [], "-", linewidth=1.5, color="blue", zorder=4)
ax2.plot([WALL_X, WALL_X], [-0.4, 0.4], "k-", linewidth=2)

def make_spring(x0, x1, n_coils=15, amp=0.08):
    """Lò xo dạng zigzag từ x0 đến x1."""
    n_seg = 2 * n_coils
    xs = np.linspace(x0, x1, n_seg + 1)
    ys = np.zeros(n_seg + 1)
    for i in range(1, n_seg):
        ys[i] = amp if i % 2 == 1 else -amp
    return xs, ys

# =========================
# 3. Chuyển động tròn đều
# =========================
ax3.set_xlim(XMIN_CIRC, XMAX_CIRC)
ax3.set_ylim(XMIN_CIRC, XMAX_CIRC + 0.1)        # hơi cao hơn để chứa chữ y
ax3.set_aspect("equal")
ax3.set_xlabel("x (cm)")
ax3.set_ylabel("y (cm)")
ax3.set_title("Chuyển động tròn đều và hình chiếu lên trục Ox")

# Quỹ đạo
theta_circle = np.linspace(0, 2 * np.pi, 500)
ax3.plot(A * np.cos(theta_circle), A * np.sin(theta_circle), "k-", linewidth=1)

# Trục
ax3.axhline(0, color="k", linewidth=0.5)
ax3.axvline(0, color="k", linewidth=0.5)
ax3.text(A + 0.15,  -0.15 * A / 4, "x", fontsize=11)
ax3.text(-0.15 * A / 4, A + 0.15,   "y", fontsize=11)
ax3.text(-0.1 * A / 4, -0.18 * A / 4, "O", fontsize=11)

# Điểm M0
M0x = A * np.cos(phi)
M0y = A * np.sin(phi)
ax3.plot(M0x, M0y, "gs", markersize=9, zorder=5)
ax3.text(M0x + 0.15, M0y + 0.15, "M₀", fontsize=11, color="green")
ax3.plot([0, M0x], [0, M0y], "g--", linewidth=0.8, alpha=0.6)

# ---- Cung φ ----
# Arc cần bán kính tính theo A; dấu của theta2 phụ thuộc φ
arc_r_phi = 0.18 * A
if phi >= 0:
    arc_phi = Arc((0, 0), 2*arc_r_phi, 2*arc_r_phi, angle=0,
                  theta1=0, theta2=np.degrees(phi), color="green", linewidth=1.6)
else:
    arc_phi = Arc((0, 0), 2*arc_r_phi, 2*arc_r_phi, angle=0,
                  theta1=np.degrees(phi), theta2=0, color="green", linewidth=1.6)
ax3.add_patch(arc_phi)
# Vị trí chữ φ: nửa đường phân giác của góc φ
phi_mid = phi / 2
ax3.text(arc_r_phi * 1.4 * np.cos(phi_mid),
         arc_r_phi * 1.4 * np.sin(phi_mid),
         "φ", fontsize=12, color="green", fontweight="bold")

# ---- Điểm M, bán kính, hình chiếu ----
radius_line,      = ax3.plot([], [], "b-", linewidth=1.6, zorder=3)
circle_point,     = ax3.plot([], [], "bo", markersize=10, zorder=6)
label_M           = ax3.text(0, 0, "M", fontsize=11, color="blue", fontweight="bold")

projection_line,  = ax3.plot([], [], "r--", linewidth=1, alpha=0.8)
projection_point, = ax3.plot([], [], "ro", markersize=8, zorder=6)
label_P           = ax3.text(0, 0, "P", fontsize=11, color="red", fontweight="bold")

op_line,          = ax3.plot([], [], "r-", linewidth=2.5, zorder=4)

# ---- Cung θ (động) ----
arc_r_theta = 0.30 * A
arc_theta = Arc((0, 0), 2*arc_r_theta, 2*arc_r_theta, angle=0,
                theta1=0, theta2=0, color="blue", linewidth=1.2, alpha=0.7)
ax3.add_patch(arc_theta)

# =========================
# Text info
# =========================
phase_text = fig.text(
    0.75, 0.5, "",
    fontsize=12,
    verticalalignment="center",
    horizontalalignment="left",
    family="monospace",
    bbox=dict(boxstyle="round,pad=0.6", facecolor="#f5f5f5", edgecolor="gray")
)

# =========================
# Animation
# =========================
def update(frame):
    time     = frame / 100.0
    theta    = omega * time + phi
    position = A * np.cos(theta)
    cx       = A * np.cos(theta)
    cy       = A * np.sin(theta)
    velocity = -A * omega * np.sin(theta)

    # Đồ thị
    point_xt.set_data([time], [position])

    # Con lắc lò xo (zigzag)
    xs, ys = make_spring(WALL_X, position, n_coils=15, amp=0.08)
    spring_line.set_data(xs, ys)
    particle.set_data([position], [0])

    # Chuyển động tròn
    circle_point.set_data([cx], [cy])
    radius_line.set_data([0, cx], [0, cy])
    projection_line.set_data([cx, cx], [cy, 0])
    projection_point.set_data([cx], [0])
    op_line.set_data([0, cx], [0, 0])

    # Nhãn M, P đặt lệch theo hướng bán kính
    r_hat = 0.12 * A / 4 + 0.05
    label_M.set_position((cx + r_hat * np.cos(theta),
                          cy + r_hat * np.sin(theta)))
    label_P.set_position((cx, -0.22 * A / 4 - 0.05))

    # Cập nhật cung θ
    theta_mod = theta % (2 * np.pi)
    arc_theta.theta1 = 0
    arc_theta.theta2 = np.degrees(theta_mod)

    # Text
    phase_text.set_text(
        f"A        = {A:6.2f} cm\n"
        f"ω        = {omega:6.3f} rad/s\n"
        f"φ        = {phi:6.3f} rad\n"
        f"T = 2π/ω = {T:6.3f} s\n"
        f"\n"
        f"─── Tại thời điểm t ───\n"
        f"t         = {time:6.2f} s\n"
        f"ωt        = {omega*time:6.2f} rad\n"
        f"θ = ωt+φ  = {theta:6.2f} rad\n"
        f"θ mod 2π  = {theta_mod:6.2f} rad\n"
        f"\n"
        f"─── Phương trình ───\n"
        f"x = A·cos(ωt+φ)\n"
        f"  = {position:6.2f} cm\n"
        f"v = -Aω·sin(ωt+φ)\n"
        f"  = {velocity:6.2f} cm/s"
    )

    return (point_xt, particle, spring_line,
            circle_point, radius_line, projection_line,
            projection_point, op_line, label_M, label_P,
            arc_theta, phase_text)

ani = FuncAnimation(
    fig, update,
    frames=400, interval=20,
    blit=False, repeat=True
)

plt.show()