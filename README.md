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

## 🛠️ Project Details

IDE: Visual studio 2010/2013

Language: C,C++.

Platform : Windows PC.

Genre : 2D action adventure


------------------------------------------------------------------------

## ▶️ How to Run

Make sure you have the following installed:
- **Visual Studio 2013**
- **MinGW Compiler** (if needed)
- **iGraphics Library** (included in this repository)


Open the project in Visual Studio 2013
- Open Visual Studio 2013.
- Go to File → Open → Project/Solution.
- Locate and select the .sln file from the cloned repository.
- Click Build → Build Solution
- Run the program by clicking Debug → Start Without Debugging
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
Project Contributors:
1. Isam Ajwad
2. Md.Anwarul Azim
3. Saadab Rashid

Special thanks to contributors and testers 💙
------------------------------------------------------------------------

## 🚀 Future Improvements

-   Settings menu (custom key bindings)
-   Better enemy AI
-   Animations & effects
-   Mobile support

## Screenshots

### **Menu**
<img src="https://github.com/mdanwarulazim/Errand-Day/blob/main/Errand%20Day/Assets/home/Menu.png" width="200" height="294">

### **Character**
<img src="https://github.com/user-attachments/assets/5c8a5fbb-6ebf-48cd-bff0-8235e79d9773" width="200" height="200">

## Youtube Link
[CSE 1200 Project: The Fallen Kingdom](https://www.youtube.com/)

## Project Report
[Project Report: The Fallen Kingdom](https://drive.google.com/drive/u/1/my-drive)
