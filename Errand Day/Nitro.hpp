#ifndef NITRO_HPP
#define NITRO_HPP

#include <cstdlib>

// ------------------------------
//          Nitro System
// ------------------------------
struct NitroPickup {
    bool active;
    int type;     // 0 = mini, 1 = max
    float x, y;
};

struct NitroSystem {
    // Meter: 0..6
    static const int STEPS = 6;

    // Pickup settings
    static const int PICKUP_CAP = 2;
    NitroPickup p[PICKUP_CAP];

    // Images
    int meterImg[STEPS + 1];
    int miniImg;
    int maxImg;
    int carNitroImg;

    // State
    bool allowed;       // only medium/hard
    bool active;        // nitro currently running
    int  level;         // 0..6

    // Drain timing (ticks of your updateGame timer)
    int drainCounter;
    int drainTicksPerStep; // e.g. 30 => one step drains every 30 updates

    // Spawn timing
    int spawnCooldown;
    int spawnMinTicks;
    int spawnRangeTicks;

    // Speed boost behavior
    double extraMaxSpeed; // allows speed to exceed cap by +extraMaxSpeed
    double accelBoost;    // adds to speed while nitro active

    // UI position + sizes (given)
    int meterX, meterY;
    int meterW, meterH;

    // Nitro car draw requirements (given)
    int nitroCarW, nitroCarH;
    int nitroYOffset;   // -35 to bottom align (204-169)

    NitroSystem() {
        allowed = false;
        active = false;
        level = 0;

        drainCounter = 0;
        drainTicksPerStep = 70;

        spawnCooldown = 0;
        spawnMinTicks = 420;
        spawnRangeTicks = 360;

        extraMaxSpeed = 15.0;
        accelBoost = 1.25;

        meterX = 608; meterY = 916;
        meterW = 60;  meterH = 132;

        nitroCarW = 157;
        nitroCarH = 204;
        nitroYOffset = -35;

        for (int i = 0; i < PICKUP_CAP; i++) p[i].active = false;
    }

    void loadImages() {
        // Meter images: nitro_0..nitro_6
        for (int i = 0; i <= STEPS; i++) {
            char path[256];
            sprintf_s(path, "Assets\\game\\nitro\\nitro_%d.png", i);
            meterImg[i] = iLoadImage(path);
        }

        // Pickups
        miniImg = iLoadImage("Assets\\game\\nitro\\nitro_mini.png");
        maxImg  = iLoadImage("Assets\\game\\nitro\\nitro_max.png");

        // Nitro car
        carNitroImg = iLoadImage("Assets\\game\\car_nitro.png");
    }

    void setAllowed(bool on) {
        allowed = on;
        if (!allowed) {
            level = 0;
            active = false;
            drainCounter = 0;
            for (int i = 0; i < PICKUP_CAP; i++) p[i].active = false;
            // keep spawnCooldown as-is; doesn't matter if disallowed
        }
    }

    void reset() {
        active = false;
        level = 0;
        drainCounter = 0;
        spawnCooldown = spawnMinTicks + (rand() % (spawnRangeTicks + 1));
        for (int i = 0; i < PICKUP_CAP; i++) p[i].active = false;
    }

    // Called when user presses N (edge-triggered in your code)
    void tryActivate() {
        if (!allowed) return;
        if (!active && level > 0) {
            active = true;
            drainCounter = 0;
        }
    }

    // Helpers for your drawGame()
    bool isActive() const { return allowed && active; }
    int  carDrawYOffset() const { return (isActive() ? nitroYOffset : 0); }
    int  carDrawH(int normalCarH) const { return (isActive() ? nitroCarH : normalCarH); }

    // Meter fill from pickup
    void applyPickup(int type) {
        if (!allowed) return;
        if (type == 0) {
            if (level < 3) level = 3;   // half fill
        } else {
            level = 6;                  // full
        }
    }

    // Update pickups + collisions + nitro drain + speed boost.
    // laneX[4] are lane anchor x positions (for car-left-aligned lanes).
    // carRect should match what you draw (include yOffset when nitro active).
    void update(
        float speed,
        const int laneX[4],
        float carX, float carY, float carW, float carH,
        double capSpeed,
        double &roadSpeed
    ) {
        if (!allowed) return;

        // --- Spawn logic ---
        if (spawnCooldown > 0) spawnCooldown--;
        if (spawnCooldown <= 0) {
            spawnOne(laneX);
            spawnCooldown = spawnMinTicks + (rand() % (spawnRangeTicks + 1));
        }

        // --- Move pickups + collision ---
        for (int i = 0; i < PICKUP_CAP; i++) {
            if (!p[i].active) continue;

            p[i].y -= speed;

            // off screen
            if (p[i].y + 85 < -20) {
                p[i].active = false;
                continue;
            }

            // collision (pickup size 56x68)
            if (rectOverlap(carX, carY, carW, carH, p[i].x, p[i].y, 70.0f, 85.0f)) {
                applyPickup(p[i].type);
                p[i].active = false;
            }
        }

        // --- Nitro speed + drain ---
        if (active) {
            double nitroMax = capSpeed + extraMaxSpeed;

            roadSpeed += accelBoost;
            if (roadSpeed > nitroMax) roadSpeed = nitroMax;

            drainCounter++;
            if (drainCounter >= drainTicksPerStep) {
                drainCounter = 0;
                level--;
                if (level <= 0) {
                    level = 0;
                    active = false;
                }
            }
        }
    }

    void drawPickups() const {
        if (!allowed) return;

        for (int i = 0; i < PICKUP_CAP; i++) {
            if (!p[i].active) continue;
            int img = (p[i].type == 0) ? miniImg : maxImg;
            iShowImage((int)p[i].x, (int)p[i].y, 70, 85, img);
        }
    }

    void drawMeter() const {
        if (!allowed) return;
        iShowImage(meterX, meterY, meterW, meterH, meterImg[level]);
    }

private:
    void spawnOne(const int laneX[4]) {
        // find free slot
        int idx = -1;
        for (int i = 0; i < PICKUP_CAP; i++) {
            if (!p[i].active) { idx = i; break; }
        }
        if (idx == -1) return;

        int lane = rand() % 4;
        int type = rand() % 2; // 0 mini, 1 max

        p[idx].active = true;
        p[idx].type = type;

        // center pickup in lane based on car lane x
        p[idx].x = (float)laneX[lane] + (157.0f / 2.0f - 70.0f / 2.0f);
        p[idx].y = 1080.0f + 80.0f;
    }

    static bool rectOverlap(float ax, float ay, float aw, float ah,
                            float bx, float by, float bw, float bh) {
        return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
    }
};

#endif