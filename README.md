# 🚗 Errand Day

A fast-paced 2D driving and combat game built with **C++ and
iGraphics**, where you fight off enemy vehicles, survive obstacles, and
complete distance-based missions.

------------------------------------------------------------------------

## 🎮 Gameplay Overview

You control a car driving through a dangerous road filled with: - Enemy
vehicles 🤖\
- Obstacles 🚧\
- Power-ups ⚡\
- Boss fights 💀

Your goal: - Travel a required **distance** - Earn enough **rage
(score)**\
- Survive long enough to complete the level

------------------------------------------------------------------------

## ⚡ Features

-   🚘 Smooth car movement with acceleration, braking, and steering\
-   🔥 Nitro boost system\
-   🪓 Bat attack system (left/right based on enemy position)\
-   🔫 Shooting system (bullets)\
-   ❤️ Health system with pickups\
-   🛡️ Shield power-up\
-   👾 Enemy AI + boss system\
-   📈 Score and high score tracking\
-   🎯 Difficulty levels (Easy / Medium / Hard)\
-   🔊 Sound effects and background music

------------------------------------------------------------------------

## 🕹️ Controls

  Action           Key
  ---------------- -------
  Move Forward     W / ↑
  Brake            S / ↓
  Move Left        A / ←
  Move Right       D / →
  Fire Bullet      Space
  Activate Nitro   M / m
  Bat Attack       L / l
  Pause            Esc
  Quit (Paused)    Q / q

------------------------------------------------------------------------

## 🎯 Objectives

Each level requires: - ✅ Reach a **distance goal** - ✅ Achieve a
**minimum score (rage requirement)**

Failing to meet both → Game Over.

------------------------------------------------------------------------

## 🧠 Game Mechanics

### 🪓 Bat System

-   Automatically swings left/right based on enemy position\
-   Knocks enemies back\
-   Can destroy enemies and drop health pickups

### 🔫 Bullet System

-   Continuous shooting with cooldown\
-   Damages enemies over time

### 🛡️ Shield

-   Temporary invulnerability\
-   Prevents all damage

### ⚡ Nitro

-   Boosts speed\
-   Disables normal acceleration temporarily

------------------------------------------------------------------------

## 🧩 Project Structure

    ├── iMain.cpp
    ├── Enemy.hpp
    ├── Obstacle.hpp
    ├── Nitro.hpp
    ├── HealthSystem.hpp
    ├── Score.hpp
    ├── HighScore.hpp
    ├── Button.hpp
    ├── Assets/
    │   ├── game/
    │   ├── hp/
    │   ├── audio/
    │   └── global/

------------------------------------------------------------------------

## 🛠️ Built With

-   **C++**
-   **iGraphics Library**
-   Windows multimedia (`mciSendString`) for audio

------------------------------------------------------------------------

## ▶️ How to Run

1.  Make sure you have **iGraphics set up**
2.  Open the project in your C++ IDE (CodeBlocks / Visual Studio)
3.  Build and run `iMain.cpp`

------------------------------------------------------------------------

## 🏆 High Scores

-   Enter your name after game over\
-   Scores are saved and displayed in the high score menu

------------------------------------------------------------------------

## 📌 Notes

-   Designed for academic/game project use\
-   Assets are organized in the `/Assets` folder\
-   Make sure paths are correct when running

------------------------------------------------------------------------

## 🙌 Credits

Developed by **Anwarul Azim, Isam Ajwad and Saadab Rashid**\
Special thanks to contributors and testers 💙

------------------------------------------------------------------------

## 🚀 Future Improvements

-   Settings menu (custom key bindings)
-   Better enemy AI
-   Animations & effects
-   Mobile support
