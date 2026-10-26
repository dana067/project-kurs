#pragma once
#pragma execution_character_set("utf-8")

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <random>
#include <optional>
#include <memory>
#include <functional>

inline sf::String U8(const std::string& s)
{
    return sf::String::fromUtf8(s.begin(), s.end());
}

inline std::string exeDir()
{
    char buf[1024] = {};
    GetModuleFileNameA(nullptr, buf, sizeof(buf));
    std::string p(buf);
    auto pos = p.find_last_of("\\/");
    return (pos != std::string::npos) ? p.substr(0, pos + 1) : "./";
}

// ── Перечисления ─────────────────────────────────────────────

enum class GameState
{
    Loading, MainMenu, Settings, Help, Rules,
    Intermission, Playing, ClosingTask,
    CatScene, Day4Choice, Ending, Pause
};

enum class Item
{
    None, BlackCoffee, CoffeeMilk, CoffeeCream, IcedCoffee, Croissant
};

enum class StationType
{
    CoffeeMachine, FridgeCombined, IceFreezer,
    CroissantCase, Trash, Door, Wall, Counter
};

// Состояние клиента
enum class ClientState
{
    WalkingIn,    // идёт от двери к кассе
    Waiting,      // стоит у кассы, ждёт заказ
    WalkingOut    // идёт от кассы к двери и исчезает
};

// ── Структуры ────────────────────────────────────────────────

struct Station
{
    StationType        type;
    sf::RectangleShape shape;
    std::string        label;
    int                unlockDay = 1;
};

struct Client
{
    sf::RectangleShape body;
    std::optional<sf::Sprite> sprite;
    std::vector<Item>  order;
    sf::Clock          msgClock;
    bool               showMsg = false;
    bool               isMale = true;
    ClientState        cstate = ClientState::WalkingIn;
    sf::Vector2f       pos;     
    sf::Vector2f       target;  
    float              speed = 120.f;
    sf::Vector2f       counterPos;
};

struct Thought
{
    std::string text;
    sf::Clock   clock;
    float       duration = 5.f;
    bool        visible = false;
};

// ── Класс Game ───────────────────────────────────────────────

class Game
{
public:
    Game();
    ~Game();
    void run();

private:
    sf::RenderWindow          window;
    sf::Font                  font;
    std::mt19937              rng;

    sf::Texture bgTex, bgRainTex, menuTex, catTex;
    sf::Texture goodEndingTex, neutralEndingTex, badEndingTex;
    sf::Texture maleFrontTex;
    sf::Texture maleBackTex;
    sf::Texture maleLeftTex;
    sf::Texture maleRightTex;

    sf::Texture femaleFrontTex;
    sf::Texture femaleBackTex;
    sf::Texture femaleLeftTex;
    sf::Texture femaleRightTex;

    sf::Texture baristaFrontTex;
    sf::Texture baristaBackTex;
    sf::Texture baristaLeftTex;
    sf::Texture baristaRightTex;
    std::optional<sf::Sprite> baristaSpr;

    sf::SoundBuffer tickBuffer;
    sf::SoundBuffer orderCompleteBuffer;
    sf::SoundBuffer coffeeBuffer;
    sf::SoundBuffer iceBuffer;
    sf::SoundBuffer croissantBuffer;
    sf::SoundBuffer milkBuffer;
    sf::Sound* tickSound = nullptr;
    sf::Sound* coffeeSound = nullptr;
    sf::Sound* iceSound = nullptr;
    sf::Sound* croissantSound = nullptr;
    sf::Sound* orderCompleteSound = nullptr;
    sf::Sound* milkSound = nullptr;

    sf::Clock clientSpawnClock;
    bool waitingForClient = false;

    sf::Music goodEndingMusic;
    sf::Music neutralEndingMusic;
    sf::Music badEndingMusic;
    sf::Music day3Music;
    sf::Music menuMusic;
    sf::Music gameMusic;
    std::optional<sf::Sprite> bgSpr, menuSpr, catSpr;
    std::optional<sf::Sprite> goodEndingSpr, neutralEndingSpr, badEndingSpr;

    GameState state = GameState::MainMenu;
    int  day = 1;
    int  wallet = 0;
    int  catKarma = 0;
    bool vetTreat = false;
    bool perfume = false;

    int  served = 0;
    int  target = 4;

    bool f_lastThought = false;
    bool f_closing = false;
    bool f_rain = false;
    bool f_rainThought = false;
    bool f_d4conflict = false;
    bool f_d5mid = false;

    int pauseSel = 0;

    int helpSel = 0;

    bool musicFading = false;
    float fadeSpeed = 18.f;
    float endingFadeSpeed = 2.f;
    bool endingMusicFade = false;

    sf::RectangleShape    player;
    float                 spd = 230.f;
    sf::Clock             dtClock;

    std::vector<Station>  stations;
    std::vector<Client>   clients;
    Item                  inv = Item::None;

    sf::Clock             dayClock, rainClock;
    Thought               thought;

    bool showFridge = false;
    int   menuSel = 0;
    float volume = 70.f;

    // ── Загрузочный экран ────────────────────────────────────
    std::vector<std::function<void()>> loadTasks;
    size_t loadTaskIndex = 0;
    std::string loadStatusText;

    // Точки маршрута клиентов
    static constexpr float DOOR_X = 75.f;   // x двери (спавн)
    static constexpr float DOOR_Y = 210.f;  // y где появляется клиент
    static constexpr float COUNTER_Y = 210.f;  // y у кассы
    // x позиции у кассы — несколько слотов
    static constexpr float COUNTER_X = 590.f;  // единственный слот (один клиент за раз)

    // ── Методы ───────────────────────────────────────────────
    void OpenHelp();
    void setupPlayer();
    void setupStations();
    void loadImage(sf::Texture& tex, std::optional<sf::Sprite>& spr, const std::string& name);

    void buildLoadQueue();
    void updateLoading();
    void drawLoading();

    void handleEvents();
    void applyVolume();
    void stopGameMusic();
    void fadeOutMusic();
    void fadeOutEndingMusic();
    void update();
    void render();

    void startDay();
    void resetDay();
    void updateMove(float dt);
    void updateTriggers();
    void updateClients(float dt);
    void checkDayEnd();
    void interact();
    void openCatScene();
    void useStation(StationType t);
    void giveItem(Client& c);
    void spawnClient();
    std::vector<Item> makeOrder();

    void onMenuKey(sf::Keyboard::Scancode sc);
    void onFridgeKey(sf::Keyboard::Scancode sc);
    void onCatKey(sf::Keyboard::Scancode sc);
    void onDay4Key(sf::Keyboard::Scancode sc);

    void drawCafe();
    void drawRain();
    void drawFridgePopup();
    void drawCatScene();
    void drawPlaying();
    void drawClosing();
    void drawDay4();
    void drawIntermission();
    void drawEnding();
    void drawMainMenu();
    void drawSettings();
    void drawRules();
    void drawHelp();
    void drawMenuBtn(const std::string& t, float x, float y, bool sel);
    void drawInventory();
    void drawOrderBubble(const Client& c);
    void drawThought();
    void drawPauseMenu();
    void drawSmallBtn(const std::string& t, float x, float y, float w, float h, bool sel);

    void        showThought(const std::string& t, float dur);
    bool        isNear(const sf::FloatRect& a, const sf::FloatRect& b) const;
    std::string itemLabel(Item i) const;
    sf::Color   itemColor(Item i) const;
    int         dayTarget(int d) const;
    void        txt(const std::string& s, float x, float y,
        unsigned sz, sf::Color c);
    void        txtWrap(const std::string& s, float x, float y,
        unsigned sz, sf::Color c, float maxW);
};
