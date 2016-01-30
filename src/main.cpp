#include <iostream>

#include "base.hpp"
#include "game.hpp"
#include "assets.hpp"
#include "battle.hpp"

// Battle system process:
// 1. Every party member selects a ritual
// 2. Ritual minigames are executed
// 3.

GGJ16_NAMESPACE
{
    enum class battle_screen_state
    {
        player_menu,
        player_ritual,
        enemy_turn
    };

    enum class ritual_minigame_state
    {
        in_progress,
        timed_out,
        success,
        invalid
    };

    class battle_ritual_context;

    namespace impl
    {
        class ritual_minigame_base
        {
        private:
            ritual_minigame_state _state{ritual_minigame_state::invalid};
            float _time_left;

        public:
            battle_ritual_context* _ctx;
            game_app& app() noexcept;

        protected:
            void time_out() { _state = ritual_minigame_state::timed_out; }
            void success() { _state = ritual_minigame_state::success; }

        public:
            const auto& state() const noexcept { return _state; }

            virtual ~ritual_minigame_base() {}

            virtual void update(ft dt)
            {
                _time_left -= dt;
                if(_time_left <= 0.f)
                {
                    time_out();
                }
            }

            virtual void draw() {}

            void start_minigame(float time_left)
            {
                _time_left = time_left;
                _state = ritual_minigame_state::in_progress;
            }

            const auto& time_left() const noexcept { return _time_left; }
        };
    }

    using ritual_uptr = std::unique_ptr<impl::ritual_minigame_base>;

    struct symbol_point
    {
        vec2f _p;
        float _radius{6.f};
    };

    class symbol_ritual : public impl::ritual_minigame_base
    {
    private:
        using base_type = impl::ritual_minigame_base;

        int next_id{1};
        vec2f _center{320 / 2.f, 240 / 2.f};
        std::vector<sf::CircleShape> _pshapes;
        std::vector<ssvs::BitmapText> _ptexts;

        auto is_shape_hovered(const sf::CircleShape& cs) noexcept
        {
            auto mp(app().window().getMousePosition());
            return ssvs::getDistEuclidean(mp, cs.getPosition()) <
                   cs.getRadius();
        }

    public:
        void add_point(const symbol_point& sp)
        {
            // Add shape
            _pshapes.emplace_back();
            auto& s(_pshapes.back());
            s.setFillColor(sfc::Red);
            s.setOutlineThickness(1);
            s.setOutlineColor(sfc::Black);
            s.setRadius(sp._radius);
            ssvs::setOrigin(s, ssvs::getLocalCenter);
            s.setPosition(_center + sp._p);

            // Add text
            _ptexts.emplace_back(mkTxtOBSmall());
            auto& t(_ptexts.back());
            t.setString(std::to_string(next_id++));
            ssvs::setOrigin(t, ssvs::getLocalCenter);
            t.setPosition(s.getPosition());
        }

        void update(ft dt) override
        {
            base_type::update(dt);

            for(auto& s : _pshapes)
            {
                if(is_shape_hovered(s))
                {
                    s.setFillColor(sfc::Green);
                }
                else
                {
                    s.setFillColor(sfc::Red);
                }
            }

            for(auto& t : _ptexts)
            {
            }
        }
        void draw() override
        {
            for(auto& s : _pshapes)
            {
                app().render(s);
            }

            for(auto& t : _ptexts)
            {
                app().render(t);
            }
        }
    };

    class example_ritual : public impl::ritual_minigame_base
    {
    private:
        using base_type = impl::ritual_minigame_base;

    public:
        void update(ft dt) override { base_type::update(dt); }
        void draw() override {}
    };

    class ritual_maker
    {
    private:
        std::string _label;
        float _time;
        std::function<ritual_uptr()> _fn_make;

    public:
        template <typename TF>
        ritual_maker(const std::string& label, float time, TF&& f)
            : _label{label}, _time{time}, _fn_make(f)
        {
        }

        const auto& time() const noexcept { return _time; }
        const auto& label() const noexcept { return _label; }

        auto make() { return _fn_make(); }
    };

    class cplayer_state
    {
    private:
        std::vector<ritual_maker> _rituals;
        // vector of available items

    public:
        cplayer_state() { _rituals.reserve(6); }

        template <typename T, typename TF, typename... Ts>
        void emplace_ritual(const std::string& label, float time, TF&& f_init)
        {
            _rituals.emplace_back(label, time, [f_init]
                {
                    auto uptr(std::make_unique<T>());
                    f_init(*uptr);
                    return uptr;
                });
        }

        template <typename TF>
        void for_rituals(TF&& f)
        {
            for(auto& r : _rituals) f(r);
        }
    };


    class battle_ritual_context
    {
    private:
        game_app& _app;
        std::unique_ptr<impl::ritual_minigame_base> _minigame{nullptr};
        ssvs::BitmapTextRich _tr{*assets().fontObStroked};
        ssvs::BTR::PtrChunk _ptr_time_text;

        auto valid() const noexcept
        {
            if(_minigame == nullptr) return false;
            if(_minigame->state() == ritual_minigame_state::invalid)
                return false;

            return true;
        }

        void init_text()
        {
            _tr.eff<BTR::Tracking>(-3)
                .eff(sfc::White)
                .in("Time left: ")
                .eff(sfc::Red)
                .in(_ptr_time_text)
                .mk("");
        }

    public:
        auto& app() noexcept { return _app; }

        battle_ritual_context(game_app& app) : _app(app) { init_text(); }

        auto is_timed_out() const noexcept
        {
            VRM_CORE_ASSERT(valid());
            return _minigame->state() == ritual_minigame_state::timed_out;
        }

        auto is_in_progress() const noexcept
        {
            VRM_CORE_ASSERT(valid());
            return _minigame->state() == ritual_minigame_state::in_progress;
        }

        auto is_success() const noexcept
        {
            VRM_CORE_ASSERT(valid());
            return _minigame->state() == ritual_minigame_state::success;
        }

        /* template <typename T, typename... Ts>
         void start_new_minigame(float time_left, Ts&&... xs)
         {
             _minigame = std::make_unique<T>(FWD(xs)...);
             _minigame->start_minigame(time_left);
         }*/

        template <typename T>
        void set_and_start_minigame(float time_as_ft, T&& x)
        {
            _minigame = std::move(x);
            _minigame->start_minigame(time_as_ft);
            _minigame->_ctx = this;
        }

        void update(ft dt)
        {
            VRM_CORE_ASSERT(valid());

            _ptr_time_text->setStr(std::to_string(
                vrmc::to_int(ssvu::getFTToSeconds(_minigame->time_left()))));

            _tr.update(dt);
            _minigame->update(dt);
        }

        void draw()
        {
            VRM_CORE_ASSERT(valid());
            _minigame->draw();

            ssvs::setOrigin(_tr, ssvs::getLocalCenterE);
            _tr.setAlign(ssvs::TextAlign::Right);
            _tr.setPosition(vec2f(320 - 20, 20));
            _app.render(_tr);
        }
    };

    namespace impl
    {
        game_app& ritual_minigame_base::app() noexcept
        {
            assert(_ctx != nullptr);
            return _ctx->app();
        }
    }

    class battle_screen;


    class battle_screen : public game_screen
    {
    public:
    private:
        struct scripted_event
        {
            float _time;
            std::function<void(float, ft)> _f_update;
            std::function<void()> _f_draw;

            void update(ft dt)
            {
                _f_update(_time, dt);
                _time -= dt;
            }
        };



        using base_type = game_screen;

        battle_menu _menu;
        battle_menu_gfx_state _menu_gfx_state;

        battle_menu_screen* _m_main;
        battle_menu_screen* _m_ritual;
        battle_menu_screen* _m_item;

        battle_context_t _battle_context;
        battle_screen_state _state{battle_screen_state::player_menu};

        battle_ritual_context _ritual_ctx;

        std::vector<scripted_event> _scripted_events;

        ssvs::BitmapTextRich _t_cs{*assets().fontObBig};
        ssvs::BTR::PtrChunk _ptr_t_cs;
        ssvs::BTR::PtrWave _ptr_t_cs_wave;

        void init_cs_text()
        {
            _scripted_events.reserve(10);

            _t_cs.eff<BTR::Tracking>(-3)
                .eff(sfc::White)
                .in(" ")
                .eff(_ptr_t_cs_wave)
                .in(_ptr_t_cs)
                .mk("");
        }

        template <typename TFU, typename TFD>
        void add_scripted_event(float time, TFU&& fu, TFD&& fd)
        {
            scripted_event se;
            se._time = ssvu::getSecondsToFT(time);
            se._f_update = fu;
            se._f_draw = fd;

            _scripted_events.emplace_back(std::move(se));
        }

        void add_scripted_text(float time, const std::string& s)
        {
            add_scripted_event(time,
                [this, time, s](float t, ft dt)
                {
                    ssvs::setOrigin(_t_cs, ssvs::getLocalCenter);
                    _t_cs.setAlign(ssvs::TextAlign::Center);
                    _t_cs.setPosition(320 / 2.f, 240 / 2.f);

                    if(t >=
                        ssvu::getSecondsToFT(time) - ssvu::getSecondsToFT(1.f))
                    {
                        _ptr_t_cs_wave->amplitude = std::max(
                            0.f, (t - ssvu::getSecondsToFT(1.f)) * 1.5f);
                    }
                    else
                    {
                        _ptr_t_cs_wave->amplitude = 0;
                    }

                    _ptr_t_cs->setStr(s);
                    _t_cs.update(dt);
                },
                [this]
                {
                    this->app().render(_t_cs);
                });
        }

        void game_over() {}
        void success() {}

        auto check_battle_state()
        {
            auto& battle(_battle_context.battle());

            if(battle.player_dead())
            {
                game_over();
                return false;
            }

            if(battle.enemy_dead())
            {
                success();
                return false;
            }

            return true;
        }

        void start_enemy_turn()
        {
            add_scripted_text(1.7f, "Enemy turn!");
            _state = battle_screen_state::enemy_turn;
        }

        void execute_ritual(ritual_maker rm)
        {
            add_scripted_text(1.7f, rm.label());

            _state = battle_screen_state::player_ritual;
            auto time_as_ft(ssvu::getSecondsToFT(rm.time()));
            _ritual_ctx.set_and_start_minigame(time_as_ft, rm.make());


            // _current_ritual = x;

            // show minigame
            // execute minigame
            // apply changes to battle

            // auto& battle(_battle_context.battle());
            // auto must_continue(check_battle_state());
            // if(!must_continue) return;
        }

        void fill_ritual_menu()
        {
            auto& m(*_m_ritual);
            m.clear();

            auto& ps(_battle_context.player_state());
            ps.for_rituals([this, &m](auto& r)
                {
                    m.emplace_choice(r.label(), [this, &r](auto& bm)
                        {
                            this->execute_ritual(r);
                            bm.pop_screen();
                        });
                });

            m.emplace_choice("Go back", [](auto& bm)
                {
                    bm.pop_screen();
                });
        }

        void fill_item_menu()
        {
            auto& m(*_m_item);
            m.clear();

            m.emplace_choice("Item 0", [this](auto& bm)
                {
                    bm.pop_screen();
                });
            m.emplace_choice("Item 1", [this](auto& bm)
                {
                    bm.pop_screen();
                });
            m.emplace_choice("Item 2", [this](auto& bm)
                {
                    bm.pop_screen();
                });
            m.emplace_choice("Go back", [](auto& bm)
                {
                    bm.pop_screen();
                });
        }

        void fill_main_menu()
        {
            auto& m(*_m_main);
            m.clear();

            m.emplace_choice("Perform ritual", [this](auto& bm)
                {
                    this->fill_ritual_menu();
                    bm.push_screen(*_m_ritual);
                });
            m.emplace_choice("Use item", [this](auto& bm)
                {
                    this->fill_item_menu();
                    bm.push_screen(*_m_item);
                });
        }

        void init_menu()
        {
            _m_main = &_menu.make_screen();
            _m_ritual = &_menu.make_screen();
            _m_item = &_menu.make_screen();

            fill_main_menu();

            _menu.on_change += [this]
            {
                _menu_gfx_state.rebuild_from(_menu);
            };

            _menu.push_screen(*_m_main);
            update_menu(1.f);
        }

        void update_menu(ft dt) { _menu_gfx_state.update(app(), _menu, dt); }
        void draw_menu() { _menu_gfx_state.draw(app().window()); }

        void update_ritual(ft dt)
        {
            _ritual_ctx.update(dt);

            if(_ritual_ctx.is_in_progress())
            {
            }
            else if(_ritual_ctx.is_timed_out())
            {
                start_enemy_turn();
            }
            else if(_ritual_ctx.is_success())
            {
                start_enemy_turn();
            }
        }
        void draw_ritual() { _ritual_ctx.draw(); }

        void update_enemy(ft) {}
        void draw_enemy() {}

        void update_scripted_events(ft dt)
        {
            VRM_CORE_ASSERT(!_scripted_events.empty());

            auto& curr(_scripted_events.front());
            curr.update(dt);

            ssvu::eraseRemoveIf(_scripted_events, [](const auto& x)
                {
                    return x._time <= 0.f;
                });
        }
        void draw_scripted_events()
        {
            VRM_CORE_ASSERT(!_scripted_events.empty());
            auto& curr(_scripted_events.front());
            curr._f_draw();
        }

        void init_battle()
        {
            auto& battle(_battle_context.battle());

            // Assume player starts
            if(battle.is_player_turn())
            {
                _state = battle_screen_state::player_menu;
            }
            else
            {
            }
        }

    public:
        battle_screen(game_app& app, battle_context_t& battle_context) noexcept
            : base_type(app),
              _battle_context{battle_context},
              _ritual_ctx{app}
        {
            init_cs_text();
            init_menu();
            init_battle();

            add_scripted_text(1.7f, "Battle start!");
        }

        void update(ft dt) override
        {
            if(!_scripted_events.empty())
            {
                update_scripted_events(dt);
                return;
            }

            if(_state == battle_screen_state::player_menu)
            {
                update_menu(dt);
            }
            else if(_state == battle_screen_state::player_ritual)
            {
                update_ritual(dt);
            }
            else if(_state == battle_screen_state::enemy_turn)
            {
                update_enemy(dt);
            }
        }

        void draw() override
        {
            if(!_scripted_events.empty())
            {
                draw_scripted_events();
                return;
            }

            if(_state == battle_screen_state::player_menu)
            {
                draw_menu();
            }
            else if(_state == battle_screen_state::player_ritual)
            {
                draw_ritual();
            }
            else if(_state == battle_screen_state::enemy_turn)
            {
                draw_enemy();
            }
        }
    };
}
GGJ16_NAMESPACE_END

int main()
{
    using namespace ggj16;

    using game_app_runner = boilerplate::app_runner<game_app>;
    game_app_runner game{"ggj2016 temp", 320, 240};
    game_app& app(game.app());

    character_stats cs0;
    cs0.health() = 100;
    cs0.shield() = 40;
    cs0.power() = 10;
    cs0.defense() = 5;
    cs0.favour() = 10;
    battle_participant b0{cs0};

    character_stats cs1;
    cs1.health() = 100;
    cs1.shield() = 40;
    cs1.power() = 10;
    cs1.defense() = 5;
    cs1.favour() = 10;
    battle_participant b1{cs1};

    cplayer_state ps;
    ps.emplace_ritual<symbol_ritual>("Fireball", 4, [](symbol_ritual& sr)
        {
            sr.add_point({{-20.f, 60.f}, 10.f});
            sr.add_point({{0.f, -60.f}, 10.f});
            sr.add_point({{20.f, 60.f}, 10.f});
            sr.add_point({{-50.f, -20.f}, 10.f});
            sr.add_point({{50.f, -20.f}, 10.f});
        });

    battle_t b{b0, b1};
    battle_context_t b_ctx{ps, b};

    auto& test_battle(app.make_screen<battle_screen>(b_ctx));
    app.push_screen(test_battle);

    game.run();
    return 0;
}

