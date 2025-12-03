/**
 * DOOMERS - Score System
 * 
 * As per description, final score is based on:
 * - Number of enemies killed
 * - Collectibles obtained  
 * - Time taken to finish both levels
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include <string>

namespace Doomers {

// ============================================================================
// Score Breakdown
// ============================================================================
struct ScoreBreakdown {
    int enemiesKilled = 0;
    int collectiblesObtained = 0;
    float timeTaken = 0;  // Seconds
    
    int baseScore = 0;
    int killBonus = 0;
    int collectibleBonus = 0;
    int timeBonus = 0;
    
    int total() const {
        return baseScore + killBonus + collectibleBonus + timeBonus;
    }
};

// ============================================================================
// Score Manager
// ============================================================================
class ScoreManager {
public:
    // Current score
    int currentScore = 0;
    
    // Stats tracking
    int enemiesKilled = 0;
    int totalEnemies = 0;
    int collectiblesObtained = 0;
    int totalCollectibles = 0;
    
    // Time tracking
    float levelStartTime = 0;
    float totalTimeTaken = 0;
    
    // Bonus values
    int killScoreValue = 100;       // Per enemy
    int collectibleScoreValue = 50;  // Per collectible
    int parTime = 300;              // Par time in seconds (5 minutes per level)
    int maxTimeBonus = 5000;        // Maximum time bonus
    
    // Level tracking
    int currentLevel = 1;
    ScoreBreakdown level1Score;
    ScoreBreakdown level2Score;
    
    void startLevel(int level) {
        currentLevel = level;
        levelStartTime = 0;
    }
    
    void update(float dt) {
        levelStartTime += dt;
    }
    
    void addKill() {
        enemiesKilled++;
        currentScore += killScoreValue;
    }
    
    void addCollectible(int bonusScore = 0) {
        collectiblesObtained++;
        currentScore += collectibleScoreValue + bonusScore;
    }
    
    void addScore(int amount) {
        currentScore += amount;
    }
    
    void finishLevel() {
        ScoreBreakdown& breakdown = (currentLevel == 1) ? level1Score : level2Score;
        
        breakdown.enemiesKilled = enemiesKilled;
        breakdown.collectiblesObtained = collectiblesObtained;
        breakdown.timeTaken = levelStartTime;
        
        // Calculate bonuses
        breakdown.killBonus = enemiesKilled * killScoreValue;
        breakdown.collectibleBonus = collectiblesObtained * collectibleScoreValue;
        
        // Time bonus - faster = more points
        if (levelStartTime < parTime) {
            float timeRatio = 1.0f - (levelStartTime / parTime);
            breakdown.timeBonus = (int)(maxTimeBonus * timeRatio);
        } else {
            breakdown.timeBonus = 0;
        }
        
        breakdown.baseScore = currentScore - breakdown.killBonus - breakdown.collectibleBonus;
        
        totalTimeTaken += levelStartTime;
        
        // Reset for next level
        if (currentLevel == 1) {
            enemiesKilled = 0;
            collectiblesObtained = 0;
            levelStartTime = 0;
            currentLevel = 2;
        }
    }
    
    int getFinalScore() const {
        return level1Score.total() + level2Score.total();
    }
    
    ScoreBreakdown getTotalBreakdown() const {
        ScoreBreakdown total;
        total.enemiesKilled = level1Score.enemiesKilled + level2Score.enemiesKilled;
        total.collectiblesObtained = level1Score.collectiblesObtained + level2Score.collectiblesObtained;
        total.timeTaken = level1Score.timeTaken + level2Score.timeTaken;
        total.baseScore = level1Score.baseScore + level2Score.baseScore;
        total.killBonus = level1Score.killBonus + level2Score.killBonus;
        total.collectibleBonus = level1Score.collectibleBonus + level2Score.collectibleBonus;
        total.timeBonus = level1Score.timeBonus + level2Score.timeBonus;
        return total;
    }
    
    // Format time as MM:SS
    static std::string formatTime(float seconds) {
        int mins = (int)(seconds / 60.0f);
        int secs = (int)seconds % 60;
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%02d:%02d", mins, secs);
        return std::string(buffer);
    }
    
    void renderHUD(int screenWidth, int screenHeight) {
        // This is called by HUD system
        // Score is displayed on screen as per description
    }
    
    void reset() {
        currentScore = 0;
        enemiesKilled = 0;
        totalEnemies = 0;
        collectiblesObtained = 0;
        totalCollectibles = 0;
        levelStartTime = 0;
        totalTimeTaken = 0;
        currentLevel = 1;
        level1Score = ScoreBreakdown();
        level2Score = ScoreBreakdown();
    }
};

} // namespace Doomers
