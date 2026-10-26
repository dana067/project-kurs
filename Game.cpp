#pragma execution_character_set("utf-8")

#include "Game.h"
#include <algorithm>
#include <cmath>
#include <windows.h>
#include <shellapi.h>

// ============================================================
//  КОНСТРУКТОР
// ============================================================

Game::Game() : window(sf::VideoMode(sf::VideoMode::getDesktopMode()), L"Кофе&Усы", sf::Style::None)
{
    sf::View view(sf::FloatRect({ 0.f, 0.f }, { 1280.f, 720.f }));
    window.setView(view);
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(false);
    window.clear(sf::Color(20, 15, 12));
    window.display();
    rng.seed(std::random_device{}());
    std::string dir = exeDir();
    font.openFromFile(dir + "EpilepsySansBold.ttf");

    state = GameState::Loading;
    buildLoadQueue();
}
Game::~Game()
{
    delete tickSound;
    delete coffeeSound;
    delete iceSound;
    delete croissantSound;
    delete orderCompleteSound;
    delete milkSound;
}

// ============================================================
//  ОЧЕРЕДЬ ЗАГРУЗКИ
// ============================================================

void Game::updateLoading()

{
    if (loadTaskIndex >= loadTasks.size())
        return;
    loadTasks[loadTaskIndex]();
    loadTaskIndex++;

}

void Game::buildLoadQueue()
{
    loadTasks.clear();
    loadTaskIndex = 0;

    loadTasks.push_back([this]()
        {
            loadStatusText = "Загрузка звуков...";
            if (milkBuffer.loadFromFile(std::filesystem::path(exeDir() + "music\\milk.ogg")))
                milkSound = new sf::Sound(milkBuffer);
            if (coffeeBuffer.loadFromFile(std::filesystem::path(exeDir() + "music\\coffee.ogg")))
                coffeeSound = new sf::Sound(coffeeBuffer);
        });

    loadTasks.push_back([this]()
        {
            if (iceBuffer.loadFromFile(std::filesystem::path(exeDir() + "music\\ice.ogg")))
                iceSound = new sf::Sound(iceBuffer);
            if (croissantBuffer.loadFromFile(std::filesystem::path(exeDir() + "music\\croissant.ogg")))
                croissantSound = new sf::Sound(croissantBuffer);
        });

    loadTasks.push_back([this]()
        {
            if (orderCompleteBuffer.loadFromFile(std::filesystem::path(exeDir() + "music\\order_complete.ogg")))
                orderCompleteSound = new sf::Sound(orderCompleteBuffer);
            if (tickBuffer.loadFromFile(std::filesystem::path(exeDir() + "music\\tick.ogg")))
                tickSound = new sf::Sound(tickBuffer);
        });

    loadTasks.push_back([this]()
        {
            loadStatusText = "Загрузка музыки...";
            day3Music.openFromFile(std::filesystem::path(exeDir() + "music\\day3_theme.ogg"));
        });

    loadTasks.push_back([this]()
        {
            if (menuMusic.openFromFile(std::filesystem::path(exeDir() + "music\\menu_theme.ogg")))
                menuMusic.setLooping(true);
            applyVolume();
        });

    loadTasks.push_back([this]()
        {
            gameMusic.openFromFile(std::filesystem::path(exeDir() + "music\\game_theme.ogg"));
            gameMusic.setLooping(true);
            gameMusic.setVolume(volume);
        });

    loadTasks.push_back([this]()
        {
            goodEndingMusic.openFromFile(
                std::filesystem::path(exeDir() + "music\\good_ending.ogg"));
            neutralEndingMusic.openFromFile(
                std::filesystem::path(exeDir() + "music\\neutral_ending.ogg"));
            badEndingMusic.openFromFile(
                std::filesystem::path(exeDir() + "music\\bad_ending.ogg"));
            goodEndingMusic.setLooping(true);
            neutralEndingMusic.setLooping(true);
            badEndingMusic.setLooping(true);
        });

    loadTasks.push_back([this]()
        {
            loadStatusText = "Загрузка изображений...";
            loadImage(bgTex, bgSpr, "images\\background11.png");
        });

    loadTasks.push_back([this]()
        {
            loadImage(menuTex, menuSpr, "images\\menu_bg.png");
        });

    loadTasks.push_back([this]()
        {
            loadImage(goodEndingTex, goodEndingSpr, "images\\good_ending.png");
        });

    loadTasks.push_back([this]()
        {
            loadImage(neutralEndingTex, neutralEndingSpr, "images\\neutral_ending.png");
        });

    loadTasks.push_back([this]()
        {
            loadImage(badEndingTex, badEndingSpr, "images\\bad_ending.png");
        });
    loadTasks.push_back([this]()
        {
            maleFrontTex.loadFromFile(exeDir() + "images\\customer_male_front.png");
            maleBackTex.loadFromFile(exeDir() + "images\\customer_male_back.png");
            maleLeftTex.loadFromFile(exeDir() + "images\\customer_male_left.png");
            maleRightTex.loadFromFile(exeDir() + "images\\customer_male_right.png");

            femaleFrontTex.loadFromFile(exeDir() + "images\\customer_female_front.png");
            femaleBackTex.loadFromFile(exeDir() + "images\\customer_female_back.png");
            femaleLeftTex.loadFromFile(exeDir() + "images\\customer_female_left.png");
            femaleRightTex.loadFromFile(exeDir() + "images\\customer_female_right.png");
        });

    loadTasks.push_back([this]()
        {
            baristaFrontTex.loadFromFile(
                exeDir() + "images\\barista_front.png");

            baristaBackTex.loadFromFile(
                exeDir() + "images\\barista_back.png");

            baristaLeftTex.loadFromFile(
                exeDir() + "images\\barista_left.png");

            baristaRightTex.loadFromFile(
                exeDir() + "images\\barista_right.png");

            baristaSpr.emplace(baristaFrontTex);

            baristaSpr->setOrigin({
                baristaFrontTex.getSize().x / 2.f,
                baristaFrontTex.getSize().y / 2.f
                });

            baristaSpr->setScale({ 1.f, 1.f });
        });
    loadTasks.push_back([this]()
        {
            bgRainTex.loadFromFile(std::filesystem::path(exeDir() + "images\\background_3.png"));
        });

    loadTasks.push_back([this]()
        {
            loadStatusText = "Подготовка кафе...";
            setupPlayer();
            setupStations();
        });

    loadTasks.push_back([this]()
        {
            loadStatusText = "Готово!";
            menuMusic.play();
            state = GameState::MainMenu;
        });
}

void Game::run()
{
    while (window.isOpen()) { handleEvents(); update(); render(); }
}

//  ===========================ЗАГРУЗКА КАРТИНОК=================================

void Game::loadImage(sf::Texture& tex, std::optional<sf::Sprite>& spr, const std::string& name)
{
    std::string path = exeDir() + name;
    if (tex.loadFromFile(path))
    {
        spr.emplace(tex);
        auto sz = tex.getSize();
        spr->setScale({ 1280.f / sz.x, 720.f / sz.y });
    }
    else
    {
        spr.reset();
    }
}
void Game::applyVolume()
{
    goodEndingMusic.setVolume(volume);
    neutralEndingMusic.setVolume(volume);
    badEndingMusic.setVolume(volume);
    day3Music.setVolume(volume);
    menuMusic.setVolume(volume);
    gameMusic.setVolume(volume);
    day3Music.setVolume(volume);

    if (milkSound) milkSound->setVolume(volume);
    if (coffeeSound) coffeeSound->setVolume(volume);
    if (iceSound) iceSound->setVolume(volume);
    if (croissantSound) croissantSound->setVolume(volume);
    if (orderCompleteSound) orderCompleteSound->setVolume(volume);
    if (tickSound) tickSound->setVolume(volume);
}
void Game::stopGameMusic()
{
    gameMusic.stop();
    day3Music.stop();
}
void Game::fadeOutMusic()
{
    musicFading = true;
}
void Game::fadeOutEndingMusic()
{
    endingMusicFade = true;
}
//  ===========================ИНИЦИАЛИЗАЦИЯ=================================
void Game::setupPlayer()
{
    player.setSize({ 36.f, 40.f });
    player.setFillColor(sf::Color::Transparent);
    player.setOutlineThickness(0.f);
    player.setPosition({ 150.f, 260.f });
    baristaSpr->setPosition(player.getPosition());
}

void Game::setupStations()
{
    stations.clear();

    auto add = [&](StationType t, float x, float y, float w, float h,
        const std::string& lbl, int day, sf::Color c)
        {
            Station s;
            s.type = t;
            s.shape.setPosition({ x, y });
            s.shape.setSize({ w, h });
            s.shape.setFillColor(sf::Color(0, 0, 0, 0));
            s.shape.setOutlineColor(c);
            s.shape.setOutlineThickness(2.f);
            s.label = lbl;
            s.unlockDay = day;
            stations.push_back(s);
        };
    add(StationType::CoffeeMachine, 687, 107, 90, 60, "", 1, sf::Color(0, 0, 0, 0));
    add(StationType::FridgeCombined, 864, 153, 119, 65, "", 2, sf::Color(0, 0, 0, 0));
    add(StationType::IceFreezer, 1005, 146, 104, 77, "", 3, sf::Color(0, 0, 0, 0));
    add(StationType::CroissantCase, 804, 224, 52, 33, "", 4, sf::Color(0, 0, 0, 0));
    add(StationType::Trash, 357, 181, 56, 30, "", 1, sf::Color(0, 0, 0, 0));
    add(StationType::Door, 149, 53, 91, 148, "", 1, sf::Color(0, 0, 0, 0));
    // ===========================стены=====================================
    add(StationType::Wall, 0, 346, 215, 54, "", 0, sf::Color(0, 0, 0, 0));//штука от комнаты персонала
    add(StationType::Wall, 0, 279, 84, 62, "", 0, sf::Color(0, 0, 0, 0));//цветок где окно
    add(StationType::Wall, 0, 390, 405, 324, "", 0, sf::Color(0, 0, 0, 0));//комната персонала
    add(StationType::Wall, 411, 135, 60, 160, "", 0, sf::Color(0, 0, 0, 0)); // барная верт
    add(StationType::Wall, 471, 220, 390, 77, "", 0, sf::Color(0, 0, 0, 0)); // барная горизонт
    add(StationType::Wall, 422, 408, 231, 59, "", 0, sf::Color(0, 0, 0, 0));//стулья 1
    add(StationType::Wall, 476, 389, 121, 106, "", 0, sf::Color(0, 0, 0, 0));//стол 1
    add(StationType::Wall, 1132, 181, 123, 39, "", 0, sf::Color(0, 0, 0, 0));//цветок справа
    add(StationType::Wall, 423, 578, 181, 56, "", 0, sf::Color(0, 0, 0, 0));//стулья 4
    add(StationType::Wall, 474, 543, 77, 105, "", 0, sf::Color(0, 0, 0, 0));//стол 4
    add(StationType::Wall, 0, 172, 5, 170, "", 0, sf::Color(0, 0, 0, 0)); //стена левая
    add(StationType::Wall, 680, 578, 234, 56, "", 0, sf::Color(0, 0, 0, 0));//стулья 5
    add(StationType::Wall, 734, 543, 122, 106, "", 0, sf::Color(0, 0, 0, 0));//стол 5
    add(StationType::Wall, 743, 410, 44, 57, "", 0, sf::Color(0, 0, 0, 0));//стулья 2
    add(StationType::Wall, 795, 390, 128, 101, "", 0, sf::Color(0, 0, 0, 0));//стол 2
    add(StationType::Wall, 1064, 295, 108, 31, "", 0, sf::Color(0, 0, 0, 0));//стулья 3_1
    add(StationType::Wall, 1036, 328, 166, 100, "", 0, sf::Color(0, 0, 0, 0));//стол 3
    add(StationType::Wall, 1064, 409, 108, 61, "", 0, sf::Color(0, 0, 0, 0));//стулья 3_2
    add(StationType::Wall, 1272, 171, 16, 757, "", 0, sf::Color(0, 0, 0, 0));//правая граница
    add(StationType::Wall, 407, 697, 1277, 17, "", 0, sf::Color(0, 0, 0, 0));//нижняя граница
    add(StationType::Counter, 468.f, 147.f, 185.f, 300.f, "", 1, sf::Color::Transparent);//касса

}

// ============================================================
//  СБРОС И СТАРТ ДНЯ
// ============================================================

void Game::resetDay()
{
    clients.clear();
    inv = Item::None;
    served = 0;
    target = dayTarget(day);
    showFridge = false;
    f_lastThought = false;
    f_closing = false;
    f_rain = false;
    f_rainThought = false;
    f_d4conflict = false;
    f_d5mid = false;
    catSpr.reset();
    gameMusic.stop();
}

void Game::startDay()
{
    menuMusic.stop();
    resetDay();
    state = GameState::Playing;
    player.setPosition({ 640.f, 480.f });
    dayClock.restart();

    if (bgSpr.has_value())
    {
        if (day == 3)
        {
            gameMusic.stop();

            day3Music.setLooping(true);
            day3Music.setVolume(volume);
            day3Music.play();

            bgSpr->setTexture(bgRainTex);
        }
        else
        {
            day3Music.stop();

            gameMusic.stop();        
            gameMusic.setLooping(true);
            gameMusic.setVolume(volume);
            gameMusic.play();     

            bgSpr->setTexture(bgTex);
        }
    }

    if (day == 5)
    {
        showThought(
            "Сегодня кошки не видно, где же она?.. Обычно я всегда вижу ее у входа...",
            6.f
        );
    }
}

int Game::dayTarget(int d) const
{
    if (d == 1) return 4;
    if (d == 2) return 5;
    if (d == 3) return 6;
    if (d == 4) return 6;
    return 7;
}

// ============================================================
//  СОБЫТИЯ
// ============================================================

void Game::handleEvents()
{
    while (const std::optional<sf::Event> ev = window.pollEvent())
    {
        if (ev->is<sf::Event::Closed>()) { window.close(); return; }
        if (const auto* kp = ev->getIf<sf::Event::KeyPressed>())
        {
            auto sc = kp->scancode;
            if (showFridge) { onFridgeKey(sc); return; }
            switch (state)
            {
            case GameState::MainMenu:
            case GameState::Settings:
            case GameState::Rules:
            case GameState::Help:
                onMenuKey(sc); break;
            case GameState::Intermission:
                startDay(); break;
            case GameState::Playing:
            case GameState::ClosingTask:
                if (sc == sf::Keyboard::Scancode::E) interact();
                if (sc == sf::Keyboard::Scancode::Escape)
                {
                    state = GameState::Pause;
                    pauseSel = 0;
                }
                break;

            case GameState::Pause:
                // УПРАВЛЕНИЕ ВНУТРИ МЕНЮ ПАУЗЫ
                if (sc == sf::Keyboard::Scancode::A)
                {
                    pauseSel = 0;
                }
                if (sc == sf::Keyboard::Scancode::D)
                {
                    pauseSel = 1;
                }
                if (sc == sf::Keyboard::Scancode::Enter)
                {
                    if (pauseSel == 0)
                    {
                        state = GameState::Playing;
                    }
                    else if (pauseSel == 1)
                    {
                        day3Music.stop();
                        gameMusic.stop();
                        menuMusic.play();
                        state = GameState::MainMenu;
                    }
                }
                if (sc == sf::Keyboard::Scancode::Escape)
                {
                    state = GameState::Playing;
                }
                break;

            case GameState::CatScene:
                onCatKey(sc); break;

            case GameState::Day4Choice:
                onDay4Key(sc); break;

            case GameState::Ending:
                if (sc == sf::Keyboard::Scancode::Enter)
                {
                    goodEndingMusic.stop();
                    neutralEndingMusic.stop();
                    badEndingMusic.stop();
                    gameMusic.stop();
                    day3Music.stop();

                    if (menuMusic.getStatus() != sf::SoundSource::Status::Playing)
                        menuMusic.play();

                    menuSel = 0;
                    state = GameState::MainMenu;
                }
                else if (sc == sf::Keyboard::Scancode::Escape)
                {
                    window.close();
                }
                break;
            }
        }
    }
}
//  ===========================UPDATE=================================
void Game::update()
{
    if (state == GameState::Loading)
    {
        updateLoading();
        return;
    }
    float dt = dtClock.restart().asSeconds();
    if (musicFading)
    {
        if (gameMusic.getStatus() == sf::SoundSource::Status::Playing)
        {
            float v = gameMusic.getVolume() - fadeSpeed * dt;
            if (v <= 0.f)
            {
                gameMusic.stop();
                gameMusic.setVolume(volume);
            }
            else
            {
                gameMusic.setVolume(v);
            }
        }
        if (day3Music.getStatus() == sf::SoundSource::Status::Playing)
        {
            float v = day3Music.getVolume() - fadeSpeed * dt;
            if (v <= 0.f)
            {
                day3Music.stop();
                day3Music.setVolume(volume);
            }
            else
            {
                day3Music.setVolume(v);
            }
        }
        if (gameMusic.getStatus() != sf::SoundSource::Status::Playing &&
            day3Music.getStatus() != sf::SoundSource::Status::Playing)
        {
            musicFading = false;

            if (state == GameState::Ending)
            {
                gameMusic.stop();
                day3Music.stop();
                if (vetTreat)
                    goodEndingMusic.play();
                else if (catKarma <= 1 && perfume)
                    badEndingMusic.play();
                else
                    neutralEndingMusic.play();
            }
        }
    }
    if (endingMusicFade)
    {
        if (gameMusic.getStatus() == sf::SoundSource::Status::Playing)
        {
            float v = gameMusic.getVolume() - endingFadeSpeed * dt;

            if (v <= 0.f)
            {
                gameMusic.stop();
                endingMusicFade = false;
            }
            else
            {
                gameMusic.setVolume(v);
            }
        }

        if (day3Music.getStatus() == sf::SoundSource::Status::Playing)
        {
            float v = day3Music.getVolume() - endingFadeSpeed * dt;

            if (v <= 0.f)
            {
                day3Music.stop();
                endingMusicFade = false;
            }
            else
            {
                day3Music.setVolume(v);
            }
        }
    }
    if (thought.visible)
    {
        if (thought.clock.getElapsedTime().asSeconds() > thought.duration)
        {
            thought.visible = false;
        }
    }
    if (state == GameState::Pause)
        return;

    if (state == GameState::Playing || state == GameState::ClosingTask)
    {
        if (!showFridge)
            updateMove(dt);
        if (state == GameState::Playing)
        {
            updateTriggers();
            updateClients(dt);
            checkDayEnd();
        }
    }
}

void Game::updateMove(float dt)
{
    sf::Vector2f dir{ 0.f, 0.f };
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) dir.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) dir.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) dir.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) dir.x += 1.f;
    if (baristaSpr)
    {
        if (dir.y < 0.f)
        {
            baristaSpr->setTexture(baristaBackTex);
        }
        else if (dir.y > 0.f)
        {
            baristaSpr->setTexture(baristaFrontTex);
        }
        else if (dir.x < 0.f)
        {
            baristaSpr->setTexture(baristaLeftTex);
        }
        else if (dir.x > 0.f)
        {
            baristaSpr->setTexture(baristaRightTex);
        }
    }
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.f) {
        dir /= len;
        sf::Vector2f movement = dir * spd * dt;

        // --- ДВИЖЕНИЕ ПО ОСИ X ---
        sf::Vector2f oldPos = player.getPosition();
        player.move({ movement.x, 0.f });

        bool collisionX = false;
        for (const auto& station : stations) {
            if (station.type == StationType::Door ||
                station.type == StationType::FridgeCombined ||
                station.type == StationType::CoffeeMachine ||
                station.type == StationType::CroissantCase ||
                station.type == StationType::Counter) {
                continue;
            }
            if (player.getGlobalBounds().findIntersection(station.shape.getGlobalBounds()).has_value()) {
                collisionX = true;
                break;
            }
        }
        if (collisionX) {
            player.setPosition(oldPos);
        }

        // --- ДВИЖЕНИЕ ПО ОСИ Y ---
        oldPos = player.getPosition();
        player.move({ 0.f, movement.y });

        bool collisionY = false;
        for (const auto& station : stations) {
            if (station.type == StationType::Door ||
                station.type == StationType::FridgeCombined ||
                station.type == StationType::CoffeeMachine ||
                station.type == StationType::CroissantCase ||
                station.type == StationType::Counter)
            {
                continue;
            }
            if (player.getGlobalBounds().findIntersection(
                station.shape.getGlobalBounds()).has_value()) {
                collisionY = true;
                break;
            }
        }
        if (collisionY) {
            player.setPosition(oldPos);
        }
    }
    auto p = player.getPosition();
    if (p.x < 0.f)    p.x = 0.f;
    if (p.x > 1244.f) p.x = 1244.f;
    if (p.y < 172.f)  p.y = 172.f;
    if (p.y > 672.f)  p.y = 672.f;
    player.setPosition(p);
    baristaSpr->setPosition(player.getPosition());
}

// ============================================================
//  СЮЖЕТНЫЕ ТРИГГЕРЫ
// ============================================================

void Game::updateTriggers()
{
    if (day != 3 && bgSpr.has_value())
    {
        bgSpr->setTexture(bgTex);
    }
    if (day == 1 && !f_lastThought && served == target - 1)
    {
        f_lastThought = true;
        showThought("Ох, ноги уже гудят под конец дня... Скорее бы закрыться", 5.f);
    }
    if (day == 3 && !f_rain && dayClock.getElapsedTime().asSeconds() > 12.f)
    {
        f_rain = true;
        rainClock.restart();
        showThought("Да уж, сегодня дождь целый день... Небо прямо прорвало!", 5.f);
    }

    if (day == 3 && f_rain && !f_rainThought && rainClock.getElapsedTime().asSeconds() > 4.f)
    {
        f_rainThought = true;
        showThought("А ведь та кошка на улице в такой сильный дождь... "
            "Она ведь промокнет насквозь! "
            "Может впустить ее на ночь, пусть спит в комнате для персонала?", 8.f);
    }

    if (day == 4 && !f_d4conflict && served >= 3)
    {
        f_d4conflict = true;
        showThought("Я все думаю о кошке... Я знаю есть ветеринарное угощение, но оно стоит столько же как и духи.. "
            "Моя мечта или её спасение?", 9.f);
    }

    if (day == 5 && !f_d5mid && served >= 3)
    {
        f_d5mid = true;
        showThought("Так много клиентов... Но из головы не выходит эта кошка. "
            "Где же она прячется?", 7.f);
    }
}

// ============================================================
//  КЛИЕНТЫ
// ============================================================

void Game::updateClients(float dt)
{
    if (clients.empty() && served < target)
    {
        if (!waitingForClient)
        {
            waitingForClient = true;
            clientSpawnClock.restart();
        }

        if (clientSpawnClock.getElapsedTime().asSeconds() >= 2.f)
        {
            spawnClient();
            waitingForClient = false;
        }
    }

    for (size_t i = 0; i < clients.size(); i++)
    {
        auto& c = clients[i];

        if (c.showMsg)
        {
            if (c.msgClock.getElapsedTime().asSeconds() > 2.f)
            {
                c.showMsg = false;
            }
        }

        if (c.cstate == ClientState::WalkingIn || c.cstate == ClientState::WalkingOut)
        {
            sf::Vector2f dir = c.target - c.pos;
            float distance = std::sqrt(dir.x * dir.x + dir.y * dir.y);

            if (distance > 3.f)
            {
                dir /= distance;
                c.pos += dir * c.speed * dt;

                c.body.setPosition(c.pos);

                if (c.sprite)
                {
                    if (std::abs(dir.y) > std::abs(dir.x))
                    {
                        if (dir.y < 0.f)
                        {
                            if (c.isMale)
                                c.sprite->setTexture(maleBackTex);
                            else
                                c.sprite->setTexture(femaleBackTex);
                        }
                        else
                        {
                            if (c.isMale)
                                c.sprite->setTexture(maleFrontTex);
                            else
                                c.sprite->setTexture(femaleFrontTex);
                        }
                    }
                    else
                    {
                        if (dir.x < 0.f)
                        {
                            if (c.isMale)
                                c.sprite->setTexture(maleLeftTex);
                            else
                                c.sprite->setTexture(femaleLeftTex);
                        }
                        else
                        {
                            if (c.isMale)
                                c.sprite->setTexture(maleRightTex);
                            else
                                c.sprite->setTexture(femaleRightTex);
                        }
                    }

                    c.sprite->setPosition(c.pos);
                }
            }
            else
            {
                if (c.cstate == ClientState::WalkingIn)
                {
                    if (c.target == sf::Vector2f(258.f, 297.f))
                    {
                        c.target = { 374.f, 351.f };
                    }
                    else if (c.target == sf::Vector2f(374.f, 351.f))
                    {
                        c.target = c.counterPos;
                    }
                    else if (c.target == c.counterPos)
                    {
                        c.cstate = ClientState::Waiting;
                        if (c.sprite)
                        {
                            if (c.isMale)
                                c.sprite->setTexture(maleBackTex);
                            else
                                c.sprite->setTexture(femaleBackTex);
                        }
                    }
                }
                else if (c.cstate == ClientState::WalkingOut)
                {
                    sf::Vector2f doorPos{ 158.f, 189.f };

                    if (c.target == sf::Vector2f(374.f, 351.f))
                    {
                        c.target = { 258.f, 297.f };
                    }
                    else if (c.target == sf::Vector2f(258.f, 297.f))
                    {
                        c.target = doorPos;
                    }
                    else if (c.target == doorPos)
                    {
                        clients.erase(clients.begin() + i);
                        i--;
                        continue;
                    }
                }
            }
        }
    }
}

void Game::spawnClient()
{
    Client c;
    c.showMsg = false;
    c.body.setSize({ 44.f, 60.f });
    c.body.setOrigin({ 22.f, 30.f });
    sf::Vector2f spawnPos{ 158.f, 189.f };
    c.body.setPosition(spawnPos);
    c.pos = spawnPos;
    std::uniform_int_distribution<int> d(0, 1);

    if (d(rng) == 0)
    {
        c.isMale = true;
        c.sprite.emplace(maleFrontTex);
    }
    else
    {
        c.isMale = false;
        c.sprite.emplace(femaleFrontTex);
    }

    c.sprite->setOrigin({
    c.sprite->getTexture().getSize().x / 2.f,
    c.sprite->getTexture().getSize().y / 2.f
        });

    c.sprite->setScale({ 1.f, 1.f });
    c.sprite->setPosition(spawnPos);
    c.body.setFillColor(sf::Color(100, 160, 210));
    c.body.setOutlineColor(sf::Color(60, 100, 150));
    c.body.setOutlineThickness(2.f);
    c.order = makeOrder();
    c.counterPos = { 539.f, 351.f };
    c.target = { 258.f, 297.f };
    c.cstate = ClientState::WalkingIn;

    clients.push_back(c);
}

std::vector<Item> Game::makeOrder()
{
    std::vector<Item> menu = { Item::BlackCoffee };
    if (day >= 2) { menu.push_back(Item::CoffeeMilk); menu.push_back(Item::CoffeeCream); }
    if (day >= 3)   menu.push_back(Item::IcedCoffee);
    if (day >= 4)   menu.push_back(Item::Croissant);

    int maxN = 1;
    if (day == 2 || day == 3) maxN = 2;
    if (day == 4)             maxN = 3;
    if (day == 5)             maxN = 4;

    std::uniform_int_distribution<int> cn(1, maxN);
    std::uniform_int_distribution<int> ci(0, (int)menu.size() - 1);

    std::vector<Item> order;
    int n = cn(rng);
    for (int i = 0; i < n; i++) order.push_back(menu[ci(rng)]);
    return order;
}

// ============================================================
//  КОНЕЦ ДНЯ
// ============================================================

void Game::checkDayEnd()
{
    if (dayClock.getElapsedTime().asSeconds() < 2.f)
        return;

    if (served >= target && clients.empty() && !f_closing)
    {
        f_closing = true;

        if (day == 5)
        {
            endingMusicFade = false;
            gameMusic.stop();
            gameMusic.setVolume(volume);
            day3Music.stop();
            day3Music.setVolume(volume);

            if (vetTreat)
                goodEndingMusic.play();
            else if (catKarma <= 1 && perfume)
                badEndingMusic.play();
            else
                neutralEndingMusic.play();

            state = GameState::Ending;
            return;
        }
        state = GameState::ClosingTask;
        showThought(
            "Смена окончена! Нужно закрыть кафе — подойди к двери и нажми [E]",
            99.f);
    }
}

// ============================================================
//  ВЗАИМОДЕЙСТВИЕ [E]
// ============================================================
void Game::giveItem(Client& c)
{
    if (inv == Item::None) return;

    auto it = std::find(c.order.begin(), c.order.end(), inv);
    if (it != c.order.end())
    {
        c.order.erase(it);
        inv = Item::None;
    }
    else
    {
        c.showMsg = true;
        c.msgClock.restart();
    }
}

void Game::interact()
{
    for (auto& s : stations)
    {
        if (s.type == StationType::Counter)
        {
            if (!isNear(player.getGlobalBounds(), s.shape.getGlobalBounds())) continue;
            if (day < s.unlockDay) continue;

            if (inv == Item::None) return;

            for (auto& c : clients)
            {
                if (c.cstate == ClientState::Waiting)
                {
                    giveItem(c);

                    if (c.order.empty())
                    {
                        if (orderCompleteSound)
                            orderCompleteSound->play();
                        served++;
                        if (day == 5 && served == target - 1)
                        {
                            fadeOutEndingMusic();
                        }
                        c.target = { 374.f, 351.f };
                        c.cstate = ClientState::WalkingOut;
                    }
                    return;
                }
            }
            return;
        }
    }
    for (auto& s : stations)
    {
        if (!isNear(player.getGlobalBounds(), s.shape.getGlobalBounds())) continue;
        if (day < s.unlockDay) continue;
        if (s.type == StationType::Door)
        {
            if (state == GameState::ClosingTask)
            {
                fadeOutMusic();

                if (day == 4)
                {
                    loadImage(catTex, catSpr,
                        "images\\cat_scene_4.png");
                    state = GameState::Day4Choice;
                }
                else
                {
                    openCatScene();
                }
            }
            return;
        }
        if (s.type == StationType::FridgeCombined)
        {
            showFridge = true;
            return;
        }
        useStation(s.type);
        return;
    }

    for (auto& c : clients)
        if (isNear(player.getGlobalBounds(), c.body.getGlobalBounds()))
        {
            giveItem(c); return;
        }
}
void Game::openCatScene()
{
    thought.visible = false;
    state = GameState::CatScene;
    loadImage(catTex, catSpr, "images\\cat_scene_" + std::to_string(day) + ".png");
}

void Game::useStation(StationType t)
{
    switch (t)
    {
    case StationType::CoffeeMachine:
        if (inv == Item::None)
        {
            inv = Item::BlackCoffee;

            if (coffeeSound)
                coffeeSound->play();
        }
        break;

    case StationType::IceFreezer:
        if (inv == Item::CoffeeMilk)
        {
            inv = Item::IcedCoffee;

            if (iceSound)
                iceSound->play();
        }
        break;

    case StationType::CroissantCase:
        if (inv == Item::None)
        {
            inv = Item::Croissant;

            if (croissantSound)
                croissantSound->play();
        }
        break;

    case StationType::Trash:
        inv = Item::None;
        break;

    default:
        break;
    }
}

// ============================================================
//  ВЫБОРЫ
// ============================================================

void Game::onFridgeKey(sf::Keyboard::Scancode sc)
{
    if (sc == sf::Keyboard::Scancode::Num1)
    {
        if (inv == Item::BlackCoffee)
        {
            inv = Item::CoffeeMilk;
            if (milkSound)
                milkSound->play();
        }

        showFridge = false;
    }
    else if (sc == sf::Keyboard::Scancode::Num2)
    {
        if (inv == Item::BlackCoffee)
        {
            inv = Item::CoffeeCream;

            if (milkSound)
                milkSound->play();
        }

        showFridge = false;
    }
    else if (sc == sf::Keyboard::Scancode::Escape)
    {
        showFridge = false;
    }
}

void Game::onCatKey(sf::Keyboard::Scancode sc)
{
    bool help = (sc == sf::Keyboard::Scancode::Num1);
    bool ignore = (sc == sf::Keyboard::Scancode::Num2);
    if (!help && !ignore)
        return;
    if (help)
        catKarma++;
    if (day >= 1 && day <= 4)
        wallet += 10;
    day++;
    resetDay();
    state = GameState::Intermission;
}

void Game::onDay4Key(sf::Keyboard::Scancode sc)
{
    if (sc == sf::Keyboard::Scancode::Num1)
    {
        vetTreat = true; perfume = false;
        wallet = 0; catKarma += 2;
    }
    else if (sc == sf::Keyboard::Scancode::Num2)
    {
        perfume = true; vetTreat = false;
        wallet = 0;
        showThought("Наконец-то они мои! Этот запах прекрасен!", 4.f);
    }
    else return;

    day++;
    resetDay();
    state = GameState::Intermission;
}

void Game::onMenuKey(sf::Keyboard::Scancode sc)
{
    // Меню настроек
    if (state == GameState::Settings)
    {
        if (sc == sf::Keyboard::Scancode::Left)
        {
            volume -= 10.f;
            if (volume < 0.f)
                volume = 0.f;

            applyVolume();
        }

        if (sc == sf::Keyboard::Scancode::Right)
        {
            volume += 10.f;
            if (volume > 100.f)
                volume = 100.f;

            applyVolume();
        }

        if (sc == sf::Keyboard::Scancode::Escape ||
            sc == sf::Keyboard::Scancode::Enter)
        {
            state = GameState::MainMenu;
        }

        return;
    }

    // Экран правил
    if (state == GameState::Rules)
    {
        if (sc == sf::Keyboard::Scancode::Escape ||
            sc == sf::Keyboard::Scancode::Enter)
        {
            state = GameState::MainMenu;
        }

        return;
    }
    // Навигация по главному меню
    if (sc == sf::Keyboard::Scancode::W)
    {
        menuSel = (menuSel - 1 + 4) % 4;
    }

    if (sc == sf::Keyboard::Scancode::S)
    {
        menuSel = (menuSel + 1) % 4;
    }

    // Переключение между "Правила игры" и "Справка"
    if (menuSel == 2)
    {
        if (sc == sf::Keyboard::Scancode::A)
        {
            helpSel = 0;
        }

        if (sc == sf::Keyboard::Scancode::D)
        {
            helpSel = 1;
        }
    }

    // Подтверждение выбора
    if (sc == sf::Keyboard::Scancode::Enter)
    {
        if (menuSel == 0)
        {
            day = 1;
            wallet = 0;
            catKarma = 0;

            vetTreat = false;
            perfume = false;

            resetDay();
            state = GameState::Intermission;
        }
        else if (menuSel == 1)
        {
            state = GameState::Settings;
        }
        else if (menuSel == 2)
        {
            if (helpSel == 0)
                state = GameState::Rules;
            else
                OpenHelp();;
        }
        else if (menuSel == 3)
        {
            window.close();
        }
    }
}

// ============================================================
//  УТИЛИТЫ
// ============================================================

std::string Game::itemLabel(Item i) const
{
    switch (i)
    {
    case Item::BlackCoffee: return "Чёрный кофе";
    case Item::CoffeeMilk:  return "Кофе с молоком";
    case Item::CoffeeCream: return "Кофе со сливками";
    case Item::IcedCoffee:  return "Айс-кофе";
    case Item::Croissant:   return "Круассан";
    default:                return "Пусто";
    }
}

sf::Color Game::itemColor(Item i) const
{
    switch (i)
    {
    case Item::BlackCoffee: return sf::Color(70, 40, 20);
    case Item::CoffeeMilk:  return sf::Color(150, 100, 70);
    case Item::CoffeeCream: return sf::Color(190, 150, 110);
    case Item::IcedCoffee:  return sf::Color(120, 180, 230);
    case Item::Croissant:   return sf::Color(230, 170, 70);
    default:                return sf::Color(40, 40, 40);
    }
}

bool Game::isNear(const sf::FloatRect& a, const sf::FloatRect& b) const
{
    sf::FloatRect ex = b;
    ex.position.x -= 15.f; ex.position.y -= 15.f;
    ex.size.x += 30.f; ex.size.y += 30.f;
    return ex.findIntersection(a).has_value();
}

void Game::showThought(const std::string& t, float dur)
{
    thought.text = t; thought.duration = dur;
    thought.visible = true; thought.clock.restart();
}

void Game::txt(const std::string& s, float x, float y, unsigned sz, sf::Color c)
{
    sf::Text t(font);
    t.setString(U8(s));
    t.setCharacterSize(sz);
    t.setFillColor(c);
    t.setPosition({ x, y });
    window.draw(t);
}

void Game::txtWrap(const std::string& s, float x, float y,
    unsigned sz, sf::Color c, float maxW)
{
    sf::Text probe(font);
    probe.setCharacterSize(sz);

    std::vector<std::string> lines;
    std::string line, word;

    auto flush = [&]()
        {
            if (word.empty()) return;
            std::string test = line.empty() ? word : line + " " + word;
            probe.setString(U8(test));
            if (probe.getLocalBounds().size.x > maxW && !line.empty())
            {
                lines.push_back(line); line = word;
            }
            else line = test;
            word.clear();
        };

    for (unsigned char ch : s)
    {
        if (ch == ' ')       flush();
        else if (ch == '\n') { flush(); lines.push_back(line); line.clear(); }
        else                 word += (char)ch;
    }
    flush();
    if (!line.empty()) lines.push_back(line);

    for (size_t i = 0; i < lines.size(); i++)
        txt(lines[i], x, y + i * (sz + 6.f), sz, c);
}

// ============================================================
//  РЕНДЕР
// ============================================================

void Game::render()
{
    window.clear(sf::Color(30, 25, 20));
    switch (state)
    {
    case GameState::Loading:       drawLoading();      break;
    case GameState::MainMenu:      drawMainMenu();     break;
    case GameState::Settings:      drawSettings();     break;
    case GameState::Intermission:  drawIntermission(); break;
    case GameState::Playing:       drawPlaying();      break;
    case GameState::ClosingTask:   drawClosing();      break;
    case GameState::CatScene:      drawCatScene();     break;
    case GameState::Day4Choice:    drawDay4();         break;
    case GameState::Ending:        drawEnding();       break;
    case GameState::Pause:         drawPauseMenu();    break;
    case GameState::Rules:         drawRules();        break;
    case GameState::Help:          drawHelp();         break;
    }
    window.display();
}
// ======================================= Загрузочный экран ========================================
void Game::drawLoading()
{
    window.clear(sf::Color(20, 15, 12));

    float progress = loadTasks.empty() ? 1.f :
        (float)loadTaskIndex / (float)loadTasks.size();
    if (progress > 1.f) progress = 1.f;

    txt("Кофе&Усы", 458.f, 230.f, 64, sf::Color(240, 200, 130));

    float barW = 600.f, barH = 26.f;
    float barX = 640.f - barW / 2.f, barY = 420.f;

    sf::RectangleShape track({ barW, barH });
    track.setPosition({ barX, barY });
    track.setFillColor(sf::Color(40, 30, 20));
    track.setOutlineColor(sf::Color(160, 110, 60));
    track.setOutlineThickness(2.f);
    window.draw(track);

    sf::RectangleShape fill({ barW * progress, barH });
    fill.setPosition({ barX, barY });
    fill.setFillColor(sf::Color(200, 140, 60));
    window.draw(fill);

    int pct = (int)(progress * 100.f);
    txt(std::to_string(pct) + "%", 640.f - 18.f, barY + barH + 12.f, 20, sf::Color(220, 185, 120));

    if (!loadStatusText.empty())
        txt(loadStatusText, 640.f - 110.f, barY - 36.f, 18, sf::Color(180, 160, 135));
}

// ======================================= Кафе ========================================
void Game::drawCafe()
{
    if (bgSpr)
    {
        window.draw(*bgSpr);
    }
    else
    {
        sf::RectangleShape fl({ 1280.f, 720.f });
        fl.setFillColor(sf::Color(185, 140, 95));
        window.draw(fl);
        sf::RectangleShape ct({ 850.f, 25.f });
        ct.setPosition({ 430.f, 198.f });
        ct.setFillColor(sf::Color(120, 75, 40));
        window.draw(ct);
    }

    for (const auto& s : stations)
    {
        if (s.type == StationType::Door) continue;
        if (day < s.unlockDay) continue;
        window.draw(s.shape);
        if (!s.label.empty())
            txt(s.label, s.shape.getPosition().x + 4.f, s.shape.getPosition().y + 4.f, 12, sf::Color(255, 220, 80, 200));
    }
}
// ===========================Игровой день ===============================
void Game::drawPlaying()
{
    drawCafe();

    for (const auto& c : clients)
    {
        if (c.sprite)
            window.draw(*c.sprite);
    }

    if (baristaSpr)
        window.draw(*baristaSpr);

    for (const auto& c : clients)
    {
        if (c.cstate == ClientState::Waiting)
            drawOrderBubble(c);
    }

    for (const auto& c : clients)
    {
        if (c.showMsg)
        {
            txt("Это не то, что я хотел...",
                c.body.getPosition().x - 70.f,
                c.body.getPosition().y - 50.f,
                14, sf::Color::Red);
        }
    }

    drawInventory();
    if (thought.visible) drawThought();
    if (showFridge) drawFridgePopup();
}

// ── Задание закрыть кафе ─────────────────────────────────────

void Game::drawClosing()
{
    drawCafe();
    if (baristaSpr)
        window.draw(*baristaSpr);

    sf::RectangleShape tb({ 360.f, 60.f });
    tb.setPosition({ 900.f, 8.f });
    tb.setFillColor(sf::Color(20, 15, 10, 215));
    tb.setOutlineColor(sf::Color(200, 160, 80));
    tb.setOutlineThickness(2.f);
    window.draw(tb);
    txt("Закрыть кафе", 916.f, 14.f, 19, sf::Color(240, 200, 100));
    txt("Подойди к двери и нажми [E]", 916.f, 38.f, 15, sf::Color(200, 200, 200));
    if (thought.visible) drawThought();
}

// ── Холодильник popup ────────────────────────────────────────

void Game::drawFridgePopup()
{
    sf::RectangleShape ov({ 1280.f, 720.f });
    ov.setFillColor(sf::Color(0, 0, 0, 110));
    window.draw(ov);

    sf::RectangleShape box({ 320.f, 170.f });
    box.setPosition({ 480.f, 275.f });
    box.setFillColor(sf::Color(245, 235, 210, 250));
    box.setOutlineColor(sf::Color(160, 110, 60));
    box.setOutlineThickness(3.f);
    window.draw(box);

    txt("Что добавить?", 500.f, 287.f, 20, sf::Color(100, 65, 35));

    sf::RectangleShape sep({ 280.f, 1.f });
    sep.setPosition({ 490.f, 315.f });
    sep.setFillColor(sf::Color(160, 110, 60, 150));
    window.draw(sep);

    bool can = (inv == Item::BlackCoffee);
    sf::Color on(50, 120, 50), off(160, 140, 120);
    txt("[1]  Молоко", 500.f, 325.f, 22, can ? on : off);
    txt("[2]  Сливки", 500.f, 360.f, 22, can ? on : off);
    txt("ESC — отмена", 500.f, 405.f, 15, sf::Color(130, 110, 85));
    if (!can)
        txt("Сначала сделай кофе!", 500.f, 425.f, 14, sf::Color(180, 80, 80));
}

// ── Сцена с кошкой ───────────────────────────────────────────

void Game::drawCatScene()
{
    window.clear(sf::Color::Black);
    window.draw(*catSpr);
}

// ── День 4 финальный выбор ───────────────────────────────────

void Game::drawDay4()
{
    window.clear(sf::Color::Black);

    if (catSpr)
    {
        window.draw(*catSpr);
    }

    sf::RectangleShape ov({ 1280.f, 720.f });
    ov.setFillColor(sf::Color(0, 0, 0, 100));
    window.draw(ov);

    sf::RectangleShape box({ 700.f, 230.f });
    box.setPosition({ 290.f, 245.f });
    box.setFillColor(sf::Color(15, 10, 8, 240));
    box.setOutlineColor(sf::Color(200, 160, 80));
    box.setOutlineThickness(3.f);
    window.draw(box);

    txt("Конец дня. В кошельке ровно 30 монет.",
        310.f, 260.f, 20, sf::Color(240, 210, 140));
    txtWrap("Кошке нужно особое ветеринарное угощение — 30 монет.\n"
        "Французские духи мечты тоже стоят ровно 30 монет.",
        310.f, 295.f, 18, sf::Color::White, 660.f);
    txt("[1]  Купить угощение для кошки", 310.f, 378.f, 21, sf::Color(120, 220, 120));
    txt("[2]  Купить духи", 310.f, 412.f, 21, sf::Color(220, 160, 100));
}

// ── Интермиссия ──────────────────────────────────────────────

void Game::drawIntermission()
{
    window.clear(sf::Color::Black);
    static int lastPlayedDay = -1;
    if (tickSound && lastPlayedDay != day)
    {
        tickSound->play();
        lastPlayedDay = day;
    }
    const char* texts[] =
    {
        "День 1.  В кошельке: 0 монет\n\nМое маленькое кафе открывает двери для посетителей.\nДенег немного, забот хватает, но я привыкла справляться со всем самостоятельно.\nСегодня кажется самым обычным днём. Однако порой одна случайная встреча способна изменить гораздо больше, чем кажется на первый взгляд...",
        "День 2.  В кошельке: 10 монет  (+10 за прошлую смену)\n\nКафе пережило свой первый рабочий день.\nЗапах кофе всё ещё витает в воздухе, в кармане звенят первые заработанные монеты.\nЯ начала копить на те французские духи из лавки за углом. Но для моей маленькой мечты нужно еще немного подождать, ведь они стоят целых 30 монет.... А это значит, пора открывать кафе и встречать новых гостей.",
        "День 3.  В кошельке: 20 монет  (+10 за прошлую смену)\n\nДо моей маленкьой мечты осталась всего одна успешная смена. Если всё пойдёт хорошо, уже совсем скоро я смогу позволить себе эту покупку.\nНо в последнее время мои мысли занимает не только это. Та самая бездомная кошка всё чаще появляется рядом с кафе. Она встречает меня по утрам, крутится неподалёку весь день и будто уже считает это место своим домом.\nИнтересно, что заставляет её постоянно возвращаться сюда?..",
        "День 4.  В кошельке: 30 монет  (+10 за прошлую смену)\n\nНаконец-то. После нескольких долгих смен у меня накопилось ровно 30 монет.\nСегодня я смогу исполнить свою маленькую мечту и купить те самые духи, о которых думала всё это время.\nОднако радость почему-то омрачает беспокойство. Бездомная кошка, которая уже несколько дней приходит к кафе, выглядит неважно. Она стала вялой, и только и сидит на заднем дворе.\nКажется, сегодня мне придётся решить, что для меня важнее...",
    };
    int idx = day - 1;
    if (idx < 0) idx = 0;
    if (idx > 4) idx = 4;
    std::string dayText;
    if (day == 5)
    {
        if (vetTreat)
        {
            dayText = "День 5.  В кошельке: 0 монет\n\nВчера я потратила все свои сбережения на особое угощение для кошки. О духах пришлось забыть, но почему-то я совсем не жалею о своём решении.\nСегодня кошка выглядит гораздо лучше. Она даже позволила мне подойти ближе, чем обычно.\nКажется, впереди нас ждёт хороший день...";
        }
        else if (perfume)
        {
            dayText = "День 5.  В кошельке: 0 монет\n\nВчера моя маленькая мечта наконец исполнилась. Французские духи оказались именно такими прекрасными, как я представляла.\nНо радость от покупки почему-то не приносит полного счастья. Кошка, которая каждый день приходила к кафе, сегодня так и не появилась.\nНадеюсь, с ней всё в порядке...";
        }
    }
    else
    {
        dayText = texts[idx];
    }
    txtWrap(dayText, 234.f, 106.f, 30, sf::Color::White, 820.f);
    txt("Нажмите любую клавишу, чтобы начать", 234.f, 580.f, 22, sf::Color(180, 180, 180));
}

// ── Концовки ─────────────────────────────────────────────────

void Game::drawEnding()
{
    window.clear(sf::Color(15, 15, 18));
    if (vetTreat)
    {
        if (goodEndingSpr)
            window.draw(*goodEndingSpr);
    }
    else if (catKarma <= 1 && perfume)
    {
        if (badEndingSpr)
            window.draw(*badEndingSpr);
    }
    else
    {
        if (neutralEndingSpr)
            window.draw(*neutralEndingSpr);
    }
    sf::RectangleShape ov({ 1280.f, 720.f });
    ov.setFillColor(sf::Color(0, 0, 0, 110));
    window.draw(ov);
    std::string title, body;
    if (vetTreat)
    {
        title = "Идеальная концовка: Котокафе";
        body = "Бариста выходит на задний двор и находит кошку "
            "с новорождёнными котятами.\n\n"
            "Вскоре кафе становится их домом. Котята носятся "
            "между столиками, радуя гостей и наполняя каждый "
            "день жизнью.\n\n"
            "Мечта о духах осталась в прошлом. Иногда счастье "
            "приходит не в красивом флаконе, а на мягких лапках.";

    }
    else if (catKarma <= 1 && perfume)
    {
        title = "Негативная концовка: Одиночество";
        body = "Кошка больше не возвращалась.\n\n"
            "Кафе жило своей обычной жизнью. "
            "Ничего плохого не произошло, но ничего и не изменилось.\n\n"
            "По крайней мере, духи оказались именно такими, "
            "как она и мечтала.";
    }
    else
    {
        title = "Нейтральная концовка: Горько-сладкая";
        body = "Кошка выжила, но стала заходить намного реже.\n\n"
            "Время от времени она заглядывала во двор вместе с котятами, "
            "словно напоминая о себе, а затем снова уходила по своим делам.\n\n"
            "Кафе осталось прежним. Но теперь в нём хранилось "
            "несколько тёплых воспоминаний.";
    }

    txt(title, 409.f, 110.f, 34, sf::Color(255, 220, 160));
    txtWrap(body, 240.f, 210.f, 26, sf::Color::White, 840.f);
    txt("Enter — в главное меню", 490.f, 640.f, 21, sf::Color(180, 180, 180));
    txt("[ESC] — выйти из игры", 490.f, 668.f, 18, sf::Color(180, 180, 180));
}

// ── Главное меню ─────────────────────────────────────────────
void Game::OpenHelp()
{
    std::string path = exeDir() + "Help\\Help.chm";
    ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_MAXIMIZE);
}
void Game::drawMainMenu()
{
    window.clear(sf::Color(20, 15, 12));

    if (menuSpr)
    {
        window.draw(*menuSpr);
        sf::RectangleShape ov({ 1280.f,720.f });
        ov.setFillColor(sf::Color(0, 0, 0, 110));
        window.draw(ov);
    }
    else
    {
        for (int i = 0; i < 12; i++)
        {
            sf::RectangleShape b({ 1280.f, 60.f });
            b.setPosition({ 0.f, i * 60.f });
            b.setFillColor(sf::Color(15 + i * 3, 10 + i * 2, 8 + i * 4));
            window.draw(b);
        }
    }

    txt("Кофе&Усы", 458.f, 140.f, 72, sf::Color(240, 200, 130));
    txt("История одного кафе", 490.f, 230.f, 26, sf::Color(180, 150, 100));

    sf::RectangleShape ln({ 400.f, 2.f });
    ln.setPosition({ 440.f, 268.f });
    ln.setFillColor(sf::Color(160, 110, 60, 180));
    window.draw(ln);

    drawMenuBtn("Новая игра", 490.f, 300.f, menuSel == 0);
    drawMenuBtn("Настройки", 490.f, 380.f, menuSel == 1);

    drawSmallBtn(
        "Правила игры",
        490.f, 470.f,
        145.f, 50.f,
        menuSel == 2 && helpSel == 0);

    drawSmallBtn(
        "Справка",
        645.f, 470.f,
        145.f, 50.f,
        menuSel == 2 && helpSel == 1);
    drawMenuBtn("Выход", 490.f, 540.f, menuSel == 3);
    txt("W/S — выбор   A/D — переключение   Enter — подтвердить", 390.f, 660.f, 18, sf::Color(180, 160, 135));
}

void Game::drawMenuBtn(const std::string& t, float x, float y, bool sel)
{
    float w = 300.f, h = 60.f;
    sf::RectangleShape btn({ w, h });
    btn.setPosition({ x, y });
    btn.setFillColor(sel ? sf::Color(160, 100, 40, 220) : sf::Color(30, 20, 12, 180));
    btn.setOutlineColor(sel ? sf::Color(240, 190, 100) : sf::Color(100, 70, 40, 150));
    btn.setOutlineThickness(sel ? 3.f : 2.f);
    window.draw(btn);

    sf::Text label(font);
    label.setString(U8(t));
    label.setCharacterSize(26);
    label.setFillColor(sel ? sf::Color(255, 235, 180) : sf::Color(200, 175, 140));
    auto bounds = label.getLocalBounds();
    label.setPosition({
        x + (w - bounds.size.x) / 2.f - bounds.position.x,
        y + (h - bounds.size.y) / 2.f - bounds.position.y
        });
    window.draw(label);
}

void Game::drawSmallBtn(const std::string& t, float x, float y, float w, float h, bool sel)
{
    sf::RectangleShape btn({ w, h });
    btn.setPosition({ x, y });
    btn.setFillColor(sel ? sf::Color(160, 100, 40, 220) : sf::Color(30, 20, 12, 180));
    btn.setOutlineColor(sel ? sf::Color(240, 190, 100) : sf::Color(100, 70, 40, 150));
    btn.setOutlineThickness(sel ? 3.f : 2.f);
    window.draw(btn);

    sf::Text label(font);
    label.setString(U8(t));
    label.setCharacterSize(18);
    label.setFillColor(sel ? sf::Color(255, 235, 180) : sf::Color(200, 175, 140));
    auto bounds = label.getLocalBounds();
    label.setPosition({
        x + (w - bounds.size.x) / 2.f - bounds.position.x,
        y + (h - bounds.size.y) / 2.f - bounds.position.y
        });
    window.draw(label);
}
void Game::drawRules()
{
    window.clear(sf::Color(20, 15, 12));

    if (menuSpr)
    {
        window.draw(*menuSpr);

        sf::RectangleShape ov({ 1280.f,720.f });
        ov.setFillColor(sf::Color(0, 0, 0, 140));
        window.draw(ov);
    }
    sf::RectangleShape pn({ 850.f, 560.f });
    pn.setPosition({ 215.f, 80.f });
    pn.setFillColor(sf::Color(25, 18, 12, 230));
    pn.setOutlineColor(sf::Color(160, 110, 60));
    pn.setOutlineThickness(3.f);
    window.draw(pn);
    txt("Правила игры", 535.f, 100.f, 36, sf::Color(240, 200, 130));
    sf::RectangleShape sp({ 780.f, 2.f });
    sp.setPosition({ 250.f, 145.f });
    sp.setFillColor(sf::Color(160, 110, 60, 150));
    window.draw(sp);
    txtWrap(
        "Добро пожаловать в кафе «Кофе&Усы»!\n\n"
        "Ваша задача — обслуживать посетителей, зарабатывать монеты за отработанные дни и принимать решения, которые повлияют на судьбу кафе и бездомной кошки.\n"
        "Управление:\n"
        "W A S D — движение\n"
        "E — взаимодействие\n"
        "ESC — пауза (также отображет статус кошелька, дня; содержит рецепты; можно выйти в главное меню)\n"
        "Приготовление заказов:\n"
        "• Кофемашина — чёрный кофе\n"
        "• Холодильник — молоко или сливки\n"
        "• Морозильник — лёд\n"
        "• Коробочка — круассан\n", 250.f,160.f, 22, sf::Color(225, 210, 190), 760.f);
    txt("ESC или Enter — назад", 543.f, 600.f, 18, sf::Color(180, 160, 135));
}
void Game::drawHelp()
{
    window.clear(sf::Color::Black);
    txt("СПРАВКА", 500.f, 250.f, 40, sf::Color::White);
    txt("ESC - назад", 500.f, 350.f, 24, sf::Color::White);
}

// ── Настройки ────────────────────────────────────────────────

void Game::drawSettings()
{
    window.clear(sf::Color(20, 15, 12));

    if (menuSpr)
    {
        window.draw(*menuSpr);
        sf::RectangleShape ov({ 1280.f,720.f });
        ov.setFillColor(sf::Color(0, 0, 0, 140));
        window.draw(ov);
    }

    sf::RectangleShape pn({ 600.f, 380.f });
    pn.setPosition({ 340.f, 170.f });
    pn.setFillColor(sf::Color(25, 18, 12, 230));
    pn.setOutlineColor(sf::Color(160, 110, 60));
    pn.setOutlineThickness(3.f);
    window.draw(pn);

    txt("Настройки", 550.f, 192.f, 36, sf::Color(240, 200, 130));

    sf::RectangleShape sp({ 540.f, 2.f });
    sp.setPosition({ 370.f, 244.f });
    sp.setFillColor(sf::Color(160, 110, 60, 150));
    window.draw(sp);

    txt("Громкость", 370.f, 285.f, 24, sf::Color(225, 210, 190));

    sf::RectangleShape vb({ 400.f, 16.f });
    vb.setPosition({ 370.f, 325.f });
    vb.setFillColor(sf::Color(50, 35, 20));
    vb.setOutlineColor(sf::Color(120, 85, 45));
    vb.setOutlineThickness(2.f);
    window.draw(vb);

    sf::RectangleShape vf({ 400.f * (volume / 100.f), 16.f });
    vf.setPosition({ 370.f, 325.f });
    vf.setFillColor(sf::Color(200, 140, 60));
    window.draw(vf);

    sf::CircleShape kn(12.f);
    kn.setFillColor(sf::Color(240, 190, 100));
    kn.setOutlineColor(sf::Color(160, 110, 50));
    kn.setOutlineThickness(2.f);
    kn.setPosition({ 370.f + 400.f * (volume / 100.f) - 12.f, 315.f });
    window.draw(kn);

    txt(std::to_string((int)volume) + "%",
        790.f, 320.f, 22, sf::Color(220, 185, 120));
    txt("Стрелки влево/вправо — изменить",
        370.f, 360.f, 16, sf::Color(180, 160, 135));
    txt("Enter — назад в меню", 550.f, 517.f, 18, sf::Color(180, 160, 135));
}

// ── UI ───────────────────────────────────────────────────────

void Game::drawInventory()
{
    sf::RectangleShape slot({ 90.f, 90.f });
    slot.setPosition({ 30.f, 600.f });
    slot.setFillColor(sf::Color(20, 20, 20));
    slot.setOutlineColor(sf::Color::White);
    slot.setOutlineThickness(3.f);
    window.draw(slot);

    txt("Руки", 42.f, 572.f, 17, sf::Color::White);

    if (inv != Item::None)
    {
        sf::CircleShape ic(28.f);
        ic.setFillColor(itemColor(inv));
        ic.setPosition({ 47.f, 617.f });
        window.draw(ic);
    }
    txt(itemLabel(inv), 140.f, 630.f, 18, sf::Color::White);
}
void Game::drawOrderBubble(const Client& c)
{
    if (c.order.empty()) return;
    float bh = 36.f + c.order.size() * 30.f;

    sf::RectangleShape bub({ 170.f, bh });
    bub.setPosition({ c.body.getPosition().x - 60.f,
                     c.body.getPosition().y - bh - 8.f });
    bub.setFillColor(sf::Color(245, 245, 245));
    bub.setOutlineColor(sf::Color::Black);
    bub.setOutlineThickness(2.f);
    window.draw(bub);

    for (size_t i = 0; i < c.order.size(); i++)
    {
        sf::CircleShape ic(11.f);
        ic.setFillColor(itemColor(c.order[i]));
        ic.setPosition({ bub.getPosition().x + 12.f,
                        bub.getPosition().y + 10.f + i * 30.f });
        window.draw(ic);
        txt(itemLabel(c.order[i]),
            bub.getPosition().x + 38.f,
            bub.getPosition().y + 6.f + i * 30.f,
            12, sf::Color::Black);
    }
}

void Game::drawThought()
{
    sf::RectangleShape bub({ 760.f, 88.f });
    bub.setPosition({ 260.f, 622.f });
    bub.setFillColor(sf::Color(255, 255, 255, 245));
    bub.setOutlineColor(sf::Color(180, 140, 100));
    bub.setOutlineThickness(2.f);
    window.draw(bub);

    sf::CircleShape t1(8.f);
    t1.setFillColor(sf::Color(255, 255, 255, 245));
    t1.setPosition({ 242.f, 638.f });
    window.draw(t1);

    sf::CircleShape t2(5.f);
    t2.setFillColor(sf::Color(255, 255, 255, 245));
    t2.setPosition({ 229.f, 652.f });
    window.draw(t2);

    txtWrap(thought.text, 278.f, 633.f, 19, sf::Color(40, 30, 20), 720.f);
}
void Game::drawPauseMenu()
{
    drawCafe();
    sf::RectangleShape ov({ 1280.f, 720.f });
    ov.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(ov);
    sf::RectangleShape panel({ 780.f, 560.f });
    panel.setPosition({ 250.f, 80.f });
    panel.setFillColor(sf::Color(22, 16, 10, 245));
    panel.setOutlineColor(sf::Color(180, 130, 60));
    panel.setOutlineThickness(3.f);
    window.draw(panel);
    txt("— Пауза —", 550.f, 100.f, 34, sf::Color(240, 200, 130));
    sf::RectangleShape sep({ 700.f, 2.f });
    sep.setPosition({ 290.f, 148.f });
    sep.setFillColor(sf::Color(160, 110, 60, 180));
    window.draw(sep);
    // ── Блок: Статус ─────────────────────────────────────────
    txt("Статус", 290.f, 160.f, 20, sf::Color(200, 170, 100));
    txt("День:       " + std::to_string(day) + " из 5",
        310.f, 188.f, 18, sf::Color(220, 210, 190));
    txt("Клиентов:   " + std::to_string(served) + " / " + std::to_string(target),
        310.f, 212.f, 18, sf::Color(220, 210, 190));
    txt("Кошелёк:    " + std::to_string(wallet) + " монет",
        310.f, 236.f, 18, sf::Color(255, 220, 120));
    sf::RectangleShape sep2({ 700.f, 1.f });
    sep2.setPosition({ 290.f, 265.f });
    sep2.setFillColor(sf::Color(120, 90, 50, 120));
    window.draw(sep2);
    // ── Блок: Управление ─────────────────────────────────────
    txt("Управление", 290.f, 275.f, 20, sf::Color(200, 170, 100));
    txt("WASD         двигаться", 310.f, 300.f, 17, sf::Color(200, 200, 180));
    txt("[E]          взаимодействие", 310.f, 322.f, 17, sf::Color(200, 200, 180));
    txt("[ESC]        открыть паузу", 310.f, 344.f, 17, sf::Color(200, 200, 180));
    sf::RectangleShape sep3({ 700.f, 1.f });
    sep3.setPosition({ 290.f, 370.f });
    sep3.setFillColor(sf::Color(120, 90, 50, 120));
    window.draw(sep3);
    // ── Блок: Рецепты ─────────────────────────────────────────
    txt("Рецепты", 290.f, 380.f, 20, sf::Color(200, 170, 100));
    // Иконки + текст рецептов
    auto drawRecipe = [&](float y, sf::Color ic, const std::string& recipe)
        {
            sf::CircleShape dot(7.f);
            dot.setFillColor(ic);
            dot.setPosition({ 312.f, y + 2.f });
            window.draw(dot);
            txt(recipe, 332.f, y, 15, sf::Color(190, 185, 175));
        };

    drawRecipe(403.f, sf::Color(70, 40, 20), "Черный кофе:      Кофемашина");
    drawRecipe(422.f, sf::Color(150, 100, 70), "Кофе с молоком:   Кофемашина -> Холодильник -> Молоко[1]");
    drawRecipe(441.f, sf::Color(190, 150, 110), "Кофе со сливками: Кофемашина -> Холодильник -> Сливки[2]");
    drawRecipe(460.f, sf::Color(120, 180, 230), "Айс-кофе:         Кофе с молоком -> Морозильник");
    drawRecipe(479.f, sf::Color(230, 170, 70), "Круассан:         Коробка на барной стойке (сбоку просто [E])");
    txt("Мусорное ведро:   подойти, [E] чтобы выбросить напиток",
        332.f, 498.f, 15, sf::Color(190, 185, 175));

    sf::RectangleShape sep4({ 700.f, 1.f });
    sep4.setPosition({ 290.f, 522.f });
    sep4.setFillColor(sf::Color(120, 90, 50, 120));
    window.draw(sep4);
    // ── Кнопки ────────────────────────────────────────────────
     // Продолжить
    {
        bool sel = (pauseSel == 0);
        float bx = 290.f, by = 532.f, bw = 300.f, bh = 44.f;
        sf::RectangleShape btn({ bw, bh });
        btn.setPosition({ bx, by });
        btn.setFillColor(sel ? sf::Color(140, 90, 30, 220) : sf::Color(30, 20, 10, 180));
        btn.setOutlineColor(sel ? sf::Color(240, 190, 80) : sf::Color(100, 70, 40, 150));
        btn.setOutlineThickness(sel ? 2.f : 1.f);
        window.draw(btn);

        sf::Text label(font);
        label.setString(U8("Продолжить"));
        label.setCharacterSize(20);
        label.setFillColor(sel ? sf::Color(255, 235, 170) : sf::Color(225, 210, 190));
        auto bounds = label.getLocalBounds();
        label.setPosition({
            bx + (bw - bounds.size.x) / 2.f - bounds.position.x,
            by + (bh - bounds.size.y) / 2.f - bounds.position.y
            });
        window.draw(label);
    }

    // Выйти в главное меню
    {
        bool sel = (pauseSel == 1);
        float bx = 650.f, by = 532.f, bw = 300.f, bh = 44.f;
        sf::RectangleShape btn({ bw, bh });
        btn.setPosition({ bx, by });
        btn.setFillColor(sel ? sf::Color(140, 90, 30, 220) : sf::Color(30, 20, 10, 180));
        btn.setOutlineColor(sel ? sf::Color(240, 190, 80) : sf::Color(100, 70, 40, 150));
        btn.setOutlineThickness(sel ? 2.f : 1.f);
        window.draw(btn);

        sf::Text label(font);
        label.setString(U8("В главное меню"));
        label.setCharacterSize(20);
        label.setFillColor(sel ? sf::Color(255, 235, 170) : sf::Color(225, 210, 190));
        auto bounds = label.getLocalBounds();
        label.setPosition({
            bx + (bw - bounds.size.x) / 2.f - bounds.position.x,
            by + (bh - bounds.size.y) / 2.f - bounds.position.y
            });
        window.draw(label);
    }
    txt("A/D — выбор   Enter — подтвердить   ESC — закрыть",
        300.f, 590.f, 15, sf::Color(180, 160, 135));
}
