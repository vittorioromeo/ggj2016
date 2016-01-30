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
        enemy_turn,
        before_enemy_turn,
        before_player_turn
    };

    enum class ritual_minigame_state
    {
        in_progress,
        failure,
        success,
        invalid
    };

    enum class ritual_type
    {
        resist,
        complete
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
            ritual_type _type{ritual_type::resist};

            battle_ritual_context* _ctx;
            game_app& app() noexcept;

            const auto& type() const noexcept { return _type; }

        protected:
            void failure() { _state = ritual_minigame_state::failure; }
            void success() { _state = ritual_minigame_state::success; }

        public:
            const auto& state() const noexcept { return _state; }

            virtual ~ritual_minigame_base() {}

            virtual void update(ft dt)
            {
                _time_left -= dt;
                if(_time_left <= 0.f)
                {
                    if(_type == ritual_type::resist)
                    {
                        success();
                    }
                    else if(_type == ritual_type::complete)
                    {
                        failure();
                    }
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
        vec2f _center{
            game_constants::width / 2.f, game_constants::height / 2.f};
        std::vector<sf::CircleShape> _pshapes;
        std::vector<ssvs::BitmapText> _ptexts;
        std::vector<int> _phits;

        auto is_shape_hovered(const sf::CircleShape& cs) noexcept
        {
            auto mp(app().mp());
            return ssvs::getDistEuclidean(mp, cs.getPosition()) <
                   cs.getRadius();
        }

        auto all_hit_before(int i)
        {
            for(int j = 0; j < i; ++j)
            {
                if(_phits[j] == 0) return false;
            }
            return true;
        }

        auto all_hit() const noexcept
        {
            for(const auto& h : _phits)
            {
                if(h == 0) return false;
            }
            return true;
        }

    public:
        void add_point(const symbol_point& sp)
        {
            // Add shape
            _pshapes.emplace_back();
            auto& s(_pshapes.back());
            s.setFillColor(sfc::Red);
            s.setOutlineThickness(2);
            s.setOutlineColor(sfc::Black);
            s.setRadius(sp._radius * 2.8f);
            ssvs::setOrigin(s, ssvs::getLocalCenter);
            s.setPosition(_center + sp._p);

            // Add text
            _ptexts.emplace_back(mkTxtOBSmall());
            auto& t(_ptexts.back());
            t.setString(std::to_string(next_id++));
            t.setScale(vec2f(3.f, 3.f));
            ssvs::setOrigin(t, ssvs::getLocalCenter);
            t.setPosition(s.getPosition());

            // Add hit
            _phits.emplace_back(0);
        }

        void update(ft dt) override
        {
            base_type::update(dt);

            if(all_hit())
            {
                success();
                return;
            }

            for(sz_t i = 0; i < _pshapes.size(); ++i)
            {
                auto& s(_pshapes[i]);
                auto& t(_ptexts[i]);
                auto& h(_phits[i]);

                if(all_hit_before(i) && is_shape_hovered(s))
                {
                    h = 1;
                }

                if(h == 1)
                {
                    s.setFillColor(sfc::Green);
                    t.setString("");
                    ssvs::setOrigin(s, ssvs::getLocalCenter);
                    s.setRadius(std::max(0.f, std::abs(s.getRadius() - (dt))));
                }
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

    class aura_ritual : public impl::ritual_minigame_base
    {
    private:
        using base_type = impl::ritual_minigame_base;

        vec2f _center{
            game_constants::width / 2.f, game_constants::height / 2.f};
        std::vector<sf::CircleShape> _pshapes;

        auto is_shape_hovered(const sf::CircleShape& cs) noexcept
        {
            auto mp(app().mp());
            return ssvs::getDistEuclidean(mp, cs.getPosition()) <
                   cs.getRadius();
        }

        auto any_dead() const noexcept
        {
            for(const auto& c : _pshapes)
                if(c.getRadius() < 5.f) return true;

            return false;
        }

    public:
        void add_point(const symbol_point& sp)
        {
            // Add shape
            _pshapes.emplace_back();
            auto& s(_pshapes.back());
            s.setFillColor(sfc::Red);
            s.setOutlineThickness(3);
            s.setOutlineColor(sfc::Black);
            s.setRadius(sp._radius * 3.1f);
            ssvs::setOrigin(s, ssvs::getLocalCenter);
            s.setPosition(_center + sp._p);
        }

        void update(ft dt) override
        {
            base_type::update(dt);

            if(any_dead())
            {
                failure();
            }

            for(sz_t i = 0; i < _pshapes.size(); ++i)
            {
                auto& s(_pshapes[i]);

                ssvs::setOrigin(s, ssvs::getLocalCenter);


                if(is_shape_hovered(s))
                {
                    s.setFillColor(sfc::Green);
                    s.setRadius(
                        std::min(98.f, std::abs(s.getRadius() + dt * 2.8f)));
                }
                else
                {
                    s.setFillColor(sfc::Red);
                    s.setRadius(
                        std::max(0.f, std::abs(s.getRadius() - (dt * 0.72f))));
                }
            }
        }
        void draw() override
        {
            for(auto& s : _pshapes)
            {
                app().render(s);
            }
        }
    };



    class drag_ritual : public impl::ritual_minigame_base
    {
    private:
        using base_type = impl::ritual_minigame_base;

        vec2f _center{
            game_constants::width / 2.f, game_constants::height / 2.f};

        std::vector<sf::RectangleShape> _ptargets;
        std::vector<int> _phits;
        std::vector<sf::CircleShape> _pdraggables;
        int _curr = -1;


        auto is_shape_hovered(const sf::RectangleShape& cs) noexcept
        {
            auto mp(app().mp());

            if(mp.x < ssvs::getGlobalLeft(cs)) return false;
            if(mp.x > ssvs::getGlobalRight(cs)) return false;

            if(mp.y < ssvs::getGlobalTop(cs)) return false;
            if(mp.y > ssvs::getGlobalBottom(cs)) return false;

            return true;
        }

        auto is_shape_hovered(const sf::CircleShape& cs) noexcept
        {
            auto mp(app().mp());
            return ssvs::getDistEuclidean(mp, cs.getPosition()) <
                   cs.getRadius();
        }

        auto is_in_target(
            const sf::CircleShape& cs, const sf::RectangleShape rs)
        {
            return ssvs::getDistEuclidean(cs.getPosition(), rs.getPosition()) <
                   20.f * 3.f;
        }

        auto all_hit() const noexcept
        {
            for(const auto& x : _phits)
                if(x == 0) return false;
            return true;
        }


    public:
        void add_target(vec2f p)
        {
            _ptargets.emplace_back();
            auto& t(_ptargets.back());

            t.setFillColor(sfc::Black);
            t.setOutlineColor(sfc::White);
            t.setOutlineThickness(10.f);
            t.setSize(vec2f(26 * 3.f, 26 * 3.f));
            ssvs::setOrigin(t, ssvs::getLocalCenter);
            t.setPosition(p);
        }

        void add_draggable(vec2f p)
        {
            _pdraggables.emplace_back();
            _phits.emplace_back(0);
            auto& t(_pdraggables.back());

            t.setFillColor(sfc::Red);
            t.setOutlineColor(sfc::Black);
            t.setOutlineThickness(4.f);
            t.setRadius(7.f * 3.f);
            ssvs::setOrigin(t, ssvs::getLocalCenter);
            t.setPosition(p);
        }

        void update(ft dt) override
        {
            base_type::update(dt);

            if(all_hit())
            {
                success();
            }


            if(_curr != -1 && _phits[_curr] == 1)
            {
                _curr = -1;
            }

            for(int i = 0; i < (int)_pdraggables.size(); ++i)
            {
                auto& s(_pdraggables[i]);
                ssvs::setOrigin(s, ssvs::getLocalCenter);

                if(_phits[i] == 0 && is_shape_hovered(s) && _curr == -1)
                {
                    s.setFillColor(sfc::Green);
                    _curr = i;
                }

                if(_curr == i && _phits[i] == 0)
                {
                    auto mp(app().mp());
                    s.setPosition(vec2f(mp));
                }

                for(auto& t : _ptargets)
                {
                    if(is_in_target(s, t))
                    {
                        s.setRadius(0.f);
                        _phits[i] = 1;
                        if(_curr == i)
                        {
                            _curr = -1;
                        }
                    }
                }
            }

            for(auto& t : _ptargets)
            {
                t.setRotation(t.getRotation() + dt * 0.5f);
            }
        }

        void draw() override
        {
            for(auto& s : _ptargets)
            {
                app().render(s);
            }

            for(auto& s : _pdraggables)
            {
                app().render(s);
            }
        }
    };

    class battle_participant;

    using ritual_effect_fn = std::function<void(battle_context_t&)>;

    class ritual_maker
    {
    private:
        std::string _label;
        ritual_type _type;
        float _time;
        std::function<ritual_uptr()> _fn_make;
        ritual_effect_fn _fn_effect;

    public:
        template <typename TF0, typename TF1>
        ritual_maker(const std::string& label, ritual_type type, float time,
            TF0&& f, TF1&& f_eff)
            : _label{label}, _type{type}, _time{time}, _fn_make(f),
              _fn_effect(f_eff)
        {
        }

        const auto& type() const noexcept { return _type; }
        const auto& time() const noexcept { return _time; }
        const auto& label() const noexcept { return _label; }

        auto make() { return _fn_make(); }
        auto& effect() { return _fn_effect; }
    };

    class battle_screen;

    struct cenemy_state
    {
        std::function<void(battle_screen&, battle_context_t&)> _f_ai;
    };

    class cplayer_state
    {
    private:
        std::vector<ritual_maker> _atk_rituals;
        std::vector<ritual_maker> _utl_rituals;
        // vector of available items

    public:
        cplayer_state()
        {
            _atk_rituals.reserve(6);
            _utl_rituals.reserve(6);
        }

        template <typename T, typename TF0, typename TF1, typename... Ts>
        void emplace_atk_ritual(const std::string& label, ritual_type rt,
            float time, TF0&& f_init, TF1&& f_effect)
        {
            _atk_rituals.emplace_back(label, rt, time,
                [f_init, rt]
                {
                    auto uptr(std::make_unique<T>());
                    uptr->_type = rt;
                    f_init(*uptr);
                    return uptr;
                },
                f_effect);
        }

        template <typename T, typename TF0, typename TF1, typename... Ts>
        void emplace_utl_ritual(const std::string& label, ritual_type rt,
            float time, TF0&& f_init, TF1&& f_effect)
        {
            _utl_rituals.emplace_back(label, rt, time,
                [f_init, rt]
                {
                    auto uptr(std::make_unique<T>());
                    uptr->_type = rt;
                    f_init(*uptr);
                    return uptr;
                },
                f_effect);
        }
        template <typename TF>
        void for_atk_rituals(TF&& f)
        {
            for(auto& r : _atk_rituals) f(r);
        }

        template <typename TF>
        void for_utl_rituals(TF&& f)
        {
            for(auto& r : _utl_rituals) f(r);
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

            _tr.setScale(vec2f(3.f, 3.f));
        }

    public:
        auto& app() noexcept { return _app; }

        battle_ritual_context(game_app& app) : _app(app) { init_text(); }

        auto is_failure() const noexcept
        {
            VRM_CORE_ASSERT(valid());
            return _minigame->state() == ritual_minigame_state::failure;
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
            _tr.setPosition(vec2f(game_constants::width - 20, 20));
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

    namespace impl
    {
        class single_stat_display
        {
        private:
            ssvs::BitmapText _t{mkTxtOBSmall()};

        public:
        };
    }

    class msgbox_screen : public game_screen
    {
    private:
        using base_type = game_screen;

        sf::RectangleShape _bg;
        ssvs::BitmapTextRich _btr{*assets().fontObStroked};
        float _safety_time{100};

        void init_bg()
        {
            _bg.setSize(vec2f(220 * 3, 120 * 3));
            _bg.setFillColor(sfc::Black);
            _bg.setOutlineColor(sfc::White);
            _bg.setOutlineThickness(3);
            ssvs::setOrigin(_bg, ssvs::getLocalCenter);
            _bg.setPosition(
                game_constants::width / 2.f, game_constants::height / 2.f);
        }

        void init_btr()
        {
            _btr.setAlign(ssvs::TextAlign::Center);
            _btr.setScale(vec2f(3.f, 3.f));
        }

    public:
        msgbox_screen(game_app& app) noexcept : base_type(app)
        {
            init_bg();
            init_btr();
        }

        auto& btr() noexcept { return _btr; }

        void update(ft dt) override
        {
            ssvs::setOrigin(_btr, ssvs::getLocalCenter);
            _btr.setPosition(vec2f(
                game_constants::width / 2.f, game_constants::height / 2.f));
            _btr.update(dt);

            if(_safety_time >= 0)
            {
                _safety_time -= dt;
            }
            else if(app().lb_down())
            {
                kill();
                app().pop_screen();
            }
        }

        void draw() override
        {
            app().render(_bg);
            app().render(_btr);
        }
    };

    class stat_bar : public sf::Transformable, public sf::Drawable
    {
    private:
        ssvs::BitmapText _txt{mkTxtOBSmall()};
        sf::RectangleShape _bar, _bar_outl;

        static constexpr float _bar_h{40.f};
        static constexpr float _bar_w_max{200.f};

    public:
        stat_bar(sf::Color bar_color)
        {
            _bar.setOutlineColor(sfc::White);
            _bar.setOutlineThickness(2);
            _bar.setFillColor(bar_color);

            _bar_outl.setOutlineColor(sfc::White);
            _bar_outl.setOutlineThickness(2);
            _bar_outl.setFillColor(sfc::Transparent);

            _txt.setScale(vec2f(3.f, 3.f));
        }

        void refresh(float value, float maxvalue)
        {
            auto width(value / maxvalue * _bar_w_max);
            _bar.setSize(vec2f(width, _bar_h));
            _bar_outl.setSize(vec2f(_bar_w_max, _bar_h));

            _txt.setString(std::to_string((int)value) + std::string{" / "} +
                           std::to_string((int)maxvalue));
            _txt.setPosition(_bar.getPosition());
        }

        virtual void draw(
            sf::RenderTarget& target, sf::RenderStates states) const override
        {
            states.transform *= getTransform();
            target.draw(_bar_outl, states);
            target.draw(_bar, states);
            target.draw(_txt, states);
        }
    };

    struct stats_gfx : public sf::Transformable, public sf::Drawable
    {
        stat_bar _health_b{sfc::Red};
        stat_bar _shield_b{sfc::Blue};

        stats_gfx() { _shield_b.setPosition(0, 60.f); }

        virtual void draw(
            sf::RenderTarget& target, sf::RenderStates states) const override
        {
            states.transform *= getTransform();
            target.draw(_health_b, states);
            target.draw(_shield_b, states);
        }

        void refresh(const character_stats& cs)
        {
            _health_b.refresh(cs.health(), cs.maxhealth());
            _shield_b.refresh(cs.shield(), cs.maxshield());
        }
    };

    class battle_screen : public game_screen
    {
    public:
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
        battle_menu_screen* _m_ritual_atk;
        battle_menu_screen* _m_ritual_utl;

        battle_context_t _battle_context;
        battle_screen_state _state{battle_screen_state::player_menu};

        battle_ritual_context _ritual_ctx;

        std::vector<scripted_event> _scripted_events;

        ssvs::BitmapTextRich _t_cs{*assets().fontObBig};
        ssvs::BTR::PtrChunk _ptr_t_cs;
        ssvs::BTR::PtrWave _ptr_t_cs_wave;

        ritual_effect_fn _success_effect;

        sf::RectangleShape _stats_bg;
        sf::RectangleShape _menu_bg;

        stats_gfx _player_stats_gfx;
        stats_gfx _enemy_stats_gfx;

        void init_menu_bg()
        {
            auto sw(350.f);
            auto h(200.f);

            _stats_bg.setSize(vec2f{sw, h});
            _stats_bg.setFillColor(sf::Color{75, 35, 35, 255});
            _stats_bg.setPosition(vec2f{0, game_constants::height - h});

            _menu_bg.setSize(vec2f{game_constants::width - sw, h});
            _menu_bg.setFillColor(sf::Color{35, 35, 35, 255});
            _menu_bg.setPosition(vec2f{sw, game_constants::height - h});

            _player_stats_gfx.setPosition(
                vec2f{20.f, game_constants::height - h + 20.f});
            _enemy_stats_gfx.setPosition(vec2f{20.f, 20.f});
        }

        void init_cs_text()
        {
            _scripted_events.reserve(10);

            _t_cs.eff<BTR::Tracking>(-3)
                .eff(sfc::White)
                .in(" ")
                .eff(_ptr_t_cs_wave)
                .in(_ptr_t_cs)
                .mk("");

            _t_cs.setScale(vec2f{3.f, 3.f});

            VRM_CORE_ASSERT(_ptr_t_cs_wave != nullptr);
            VRM_CORE_ASSERT(_ptr_t_cs != nullptr);
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
                    _t_cs.setPosition(game_constants::width / 2.f,
                        game_constants::height / 2.f);

                    VRM_CORE_ASSERT(_ptr_t_cs_wave != nullptr);
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


                    VRM_CORE_ASSERT(_ptr_t_cs != nullptr);
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
            _state = battle_screen_state::before_enemy_turn;
        }

        void end_enemy_turn()
        {
            _state = battle_screen_state::before_player_turn;
        }

        void execute_ritual(ritual_maker rm)
        {
            add_scripted_text(1.7f, rm.label());

            _success_effect = rm.effect();
            _state = battle_screen_state::player_ritual;
            auto time_as_ft(ssvu::getSecondsToFT(rm.time()));
            _ritual_ctx.set_and_start_minigame(time_as_ft, rm.make());
        }

        void fill_attack_menu()
        {
            auto& m(*_m_ritual_atk);
            m.clear();

            auto& ps(_battle_context.player_state());
            ps.for_atk_rituals([this, &m](auto& rr)
                {
                    m.emplace_choice(rr.label(), [this, &rr](auto& bm)
                        {
                            this->execute_ritual(rr);
                            bm.pop_screen();
                        });
                });

            m.emplace_choice("Go back", [](auto& bm)
                {
                    bm.pop_screen();
                });
        }

        void fill_utility_menu()
        {
            auto& m(*_m_ritual_utl);
            m.clear();

            auto& ps(_battle_context.player_state());
            ps.for_utl_rituals([this, &m](auto& rr)
                {
                    m.emplace_choice(rr.label(), [this, &rr](auto& bm)
                        {
                            this->execute_ritual(rr);
                            bm.pop_screen();
                        });
                });

            m.emplace_choice("Go back", [](auto& bm)
                {
                    bm.pop_screen();
                });
        }

        void display_msg_box(const std::string& s)
        {
            auto& mbs(app().make_screen<msgbox_screen>());
            auto& btr(mbs.btr());

            btr.eff<BTR::Tracking>(-3).eff(sfc::White).in(s);
            app().push_screen(mbs);
        }

        void fill_main_menu()
        {
            auto& m(*_m_main);
            m.clear();

            m.emplace_choice("Attack rituals", [this](auto& bm)
                {
                    this->fill_attack_menu();
                    bm.push_screen(*_m_ritual_atk);
                });
            m.emplace_choice("Utility rituals", [this](auto& bm)
                {
                    this->fill_utility_menu();
                    bm.push_screen(*_m_ritual_utl);
                });
            m.emplace_choice("Inspect enemy", [this](auto&)
                {
                    const auto& es(_battle_context.enemy().stats());

                    std::ostringstream oss;
                    oss << "Inspecting enemy...\n\n";
                    oss << "Health: " << es.health() << " / " << es.maxhealth()
                        << "\n";
                    oss << "Shield: " << es.shield() << " / " << es.maxshield()
                        << "\n";
                    oss << "Power: " << es.power() << "\n";

                    this->display_msg_box(oss.str());
                });
        }

        void init_menu()
        {
            _m_main = &_menu.make_screen();
            _m_ritual_atk = &_menu.make_screen();
            _m_ritual_utl = &_menu.make_screen();

            fill_main_menu();

            _menu.on_change += [this]
            {
                _menu_gfx_state.rebuild_from(_menu);
            };

            _menu.push_screen(*_m_main);
            update_menu(1.f);
        }

        void ritual_failure()
        {
            app()._shake = 40;
            add_scripted_text(1.1f, "Failure!");
        }
        void ritual_success()
        {
            add_scripted_text(1.1f, "Success!");

            _success_effect(_battle_context);
        }


        void update_stat_bars()
        {
            _player_stats_gfx.refresh(_battle_context.player().stats());
            _enemy_stats_gfx.refresh(_battle_context.enemy().stats());
        }


        void update_menu(ft dt) { _menu_gfx_state.update(app(), _menu, dt); }
        void draw_menu()
        {
            app().render(_menu_bg);
            _menu_gfx_state.draw(app().window());
        }

        void draw_stats_bars()
        {
            app().render(_stats_bg);
            app().render(_player_stats_gfx);
            app().render(_enemy_stats_gfx);
        }

        void update_ritual(ft dt)
        {
            _ritual_ctx.update(dt);

            if(_ritual_ctx.is_in_progress())
            {
            }
            else if(_ritual_ctx.is_failure())
            {

                ritual_failure();
                start_enemy_turn();
            }
            else if(_ritual_ctx.is_success())
            {
                ritual_success();
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

        auto m_player_name() const { return std::string{"The player "}; }
        auto m_enemy_name() const { return std::string{"The enemy "}; }

        auto m_health(stat_value x) const
        {
            return std::string{" "} + std::to_string((int)x) +
                   std::string{" health points"};
        }

        auto m_shield(stat_value x) const
        {
            return std::string{" "} + std::to_string((int)x) +
                   std::string{" shield points"};
        }

    private:
        std::vector<std::string> _next_notifications;

    public:
        void append_turn_notification(const std::string& s)
        {
            _next_notifications.emplace_back(s + ".");
        }

        void event_listener(battle_event be)
        {
            auto apnd_dmg = [this](auto&& name_fn, auto&& my_be)
            {
                append_turn_notification(
                    name_fn() + "was damaged for\n" +
                    m_health((int)(my_be.e_damage()._amount)));
            };

            auto apnd_sdmg = [this](auto&& name_fn, auto&& my_be)
            {
                append_turn_notification(
                    name_fn() + "shield was damaged for\n" +
                    m_shield((int)(my_be.e_damage()._amount)));
            };

            auto apnd_hl = [this](auto&& name_fn, auto&& my_be)
            {
                append_turn_notification(
                    name_fn() + "was healed for\n" +
                    m_health((int)(my_be.e_heal()._amount)));
            };

            auto apnd_shl = [this](auto&& name_fn, auto&& my_be)
            {
                append_turn_notification(
                    name_fn() + "shield was restored for\n" +
                    m_shield((int)(my_be.e_heal()._amount)));
            };

            auto apnd_stn = [this](auto&& name_fn, auto&& my_be)
            {
                append_turn_notification(
                    name_fn() + "was stunned for for\n" +
                    m_shield((int)(my_be.e_stun()._turns)) + " turns");
            };

            switch(be.type())
            {
                case battle_event_type::enemy_damaged:
                    apnd_dmg(
                        [this]
                        {
                            return m_enemy_name();
                        },
                        be);
                    break;

                case battle_event_type::player_damaged:
                    apnd_dmg(
                        [this]
                        {
                            return m_player_name();
                        },
                        be);
                    break;

                case battle_event_type::enemy_shield_damaged:
                    apnd_sdmg(
                        [this]
                        {
                            return m_enemy_name();
                        },
                        be);
                    break;

                case battle_event_type::player_shield_damaged:
                    apnd_sdmg(
                        [this]
                        {
                            return m_player_name();
                        },
                        be);
                    break;

                case battle_event_type::enemy_healed:
                    apnd_hl(
                        [this]
                        {
                            return m_enemy_name();
                        },
                        be);
                    break;

                case battle_event_type::player_healed:
                    apnd_hl(
                        [this]
                        {
                            return m_player_name();
                        },
                        be);
                    break;

                case battle_event_type::enemy_shield_healed:
                    apnd_shl(
                        [this]
                        {
                            return m_enemy_name();
                        },
                        be);
                    break;

                case battle_event_type::player_shield_healed:
                    apnd_shl(
                        [this]
                        {
                            return m_player_name();
                        },
                        be);
                    break;

                case battle_event_type::enemy_stunned:
                    apnd_stn(
                        [this]
                        {
                            return m_enemy_name();
                        },
                        be);
                    break;
            }
        }

        void init_battle()
        {
            auto& battle(_battle_context.battle());
            _battle_context.on_event += [this](auto e)
            {
                this->event_listener(e);
            };

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
            init_menu_bg();
            init_cs_text();
            init_menu();
            init_battle();

            add_scripted_text(1.7f, "Battle start!");
        }

        void update(ft dt) override
        {
            update_stat_bars();

            if(!_scripted_events.empty())
            {
                update_scripted_events(dt);
                return;
            }

            if(!_next_notifications.empty())
            {
                std::string acc;
                for(const auto& nn : _next_notifications) acc += nn + "\n";
                display_msg_box(acc);

                _next_notifications.clear();
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
                // update_enemy(dt);
                _battle_context.enemy_state()._f_ai(*this, _battle_context);
            }
            else if(_state == battle_screen_state::before_enemy_turn)
            {
                add_scripted_text(1.7f, "Enemy turn!");
                _state = battle_screen_state::enemy_turn;
            }
            else if(_state == battle_screen_state::before_player_turn)
            {
                add_scripted_text(1.7f, "Player turn!");
                _state = battle_screen_state::player_menu;
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
                draw_stats_bars();
            }
            else if(_state == battle_screen_state::player_ritual)
            {
                draw_ritual();
            }
            else if(_state == battle_screen_state::enemy_turn)
            {
                draw_enemy();
                draw_stats_bars();
            }
        }
    };
}
GGJ16_NAMESPACE_END

void fill_ps(ggj16::cplayer_state& ps)
{
    using namespace ggj16;

    ps.emplace_atk_ritual<symbol_ritual>("Fireball", ritual_type::complete, 4,
        [](symbol_ritual& sr)
        {
            sr.add_point({{-30.f * 2.8f, 60.f * 2.8f}, 10.f});
            sr.add_point({{0.f * 2.8f, -60.f * 2.8f}, 10.f});
            sr.add_point({{30.f * 2.8f, 60.f * 2.8f}, 10.f});
            sr.add_point({{-50.f * 2.8f, -10.f * 2.8f}, 10.f});
            sr.add_point({{50.f * 2.8f, -10.f * 2.8f}, 10.f});
        },
        [](battle_context_t& c)
        {
            c.damage_enemy_by(20);
        });

    ps.emplace_atk_ritual<aura_ritual>("Rend shield", ritual_type::resist, 6,
        [](aura_ritual& sr)
        {
            sr.add_point({{-0.f, -45.f * 2.8f}, 20.f});
            sr.add_point({{-31.f * 2.8f, 31.f * 2.8f}, 20.f});
            sr.add_point({{31.f * 2.8f, 31.f * 2.8f}, 20.f});
        },
        [](battle_context_t& c)
        {
            c.damage_enemy_by(5);
            c.damage_enemy_shield_by(25);
        });

    ps.emplace_atk_ritual<symbol_ritual>("Obliterate", ritual_type::complete, 3,
        [](symbol_ritual& sr)
        {
            auto x(-1024 / 2.f);
            auto y(-768 / 2.f);
            int x_count = 6;
            int y_count = 3;
            auto offset = 40.f;

            auto fullx = game_constants::width - offset * 2;
            auto fully = game_constants::height - offset * 2;

            auto xinc(fullx / x_count);
            auto yinc(fully / y_count);


            for(int ix = 0; ix < x_count; ++ix)
            {
                sr.add_point({{x + offset + xinc * ix, y + offset}, 12.f});
            }

            for(int iy = 0; iy < y_count + 1; ++iy)
            {
                sr.add_point(
                    {{x + offset + fullx, y + offset + yinc * iy}, 12.f});
            }


            for(int ix = x_count - 1; ix >= 0; --ix)
            {
                sr.add_point({{x + offset + xinc * ix,
                                  y + game_constants::height - offset},
                    12.f});
            }

            for(int iy = y_count - 1; iy > 0; --iy)
            {
                sr.add_point({{x + offset, y + offset + yinc * iy}, 12.f});
            }
        },
        [](battle_context_t& c)
        {
            c.damage_enemy_shield_by(10);
            c.damage_enemy_by(60);
        });

    ps.emplace_utl_ritual<drag_ritual>("Heal", ritual_type::complete, 5,
        [](drag_ritual& sr)
        {
            sr.add_target(
                {game_constants::width / 2.f, game_constants::height / 2.f});

            for(int i = 0; i < 6; ++i)
            {
                auto offset(20.f * 2.8f);
                auto x(ssvu::getRndR(offset, game_constants::width - offset));
                auto y(ssvu::getRndR(offset, game_constants::height - offset));
                sr.add_draggable(vec2f{x, y});
            }
        },
        [](battle_context_t& c)
        {
            c.damage_player_shield_by(15);
            c.heal_player_by(75);
        });

    ps.emplace_utl_ritual<drag_ritual>("Repair shield", ritual_type::complete,
        6,
        [](drag_ritual& sr)
        {
            sr.add_target(
                {game_constants::width / 2.f, game_constants::height / 2.f});

            for(int i = 0; i < 3; ++i)
            {
                auto offset(20.f * 2.8f);
                sr.add_draggable(vec2f{offset, offset + (i * 60)});
                sr.add_draggable(
                    vec2f{game_constants::width - offset, offset + (i * 60)});
            }
        },
        [](battle_context_t& c)
        {
            c.damage_player_by(5);
            c.heal_player_shield_by(45);
        });
}


int main()
{
    using namespace ggj16;

    using game_app_runner = boilerplate::app_runner<game_app>;
    game_app_runner game{
        "ggj2016 temp", game_constants::width, game_constants::height};
    game_app& app(game.app());

    character_stats cs0;
    cs0.health() = 100;
    cs0.shield() = 40;
    cs0.power() = 10;
    cs0.maxhealth() = 100;
    cs0.maxshield() = 40;
    battle_participant b0{cs0};

    character_stats cs1;
    cs1.health() = 100;
    cs1.shield() = 40;
    cs1.power() = 10;
    cs1.maxhealth() = 100;
    cs1.maxshield() = 40;
    battle_participant b1{cs1};

    cplayer_state ps;
    fill_ps(ps);

    cenemy_state es;
    es._f_ai = [](battle_screen& bs, battle_context_t& bc)
    {
        bs.display_msg_box("Test");
        bc.damage_player_by(10);
        bs.end_enemy_turn();
    };

    battle_t b{b0, b1};
    battle_context_t b_ctx{ps, es, b};

    auto& test_battle(app.make_screen<battle_screen>(b_ctx));
    app.push_screen(test_battle);

    game.run();
    return 0;
}
