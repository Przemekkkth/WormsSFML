linux {
    CONFIG += link_pkgconfig
    PKGCONFIG += sfml-window sfml-graphics sfml-system sfml-audio
}


HEADERS += \
    src/SFX/music_player.h \
    src/SFX/sound_player.h \
    src/application.h \
    src/const/constants.h \
    src/const/state_identifiers.h \
    src/entity/debris.h \
    src/entity/dummy.h \
    src/entity/missile.h \
    src/entity/physics_object.h \
    src/entity/worm.h \
    src/player.h \
    src/states/game_state.h \
    src/states/state.h \
    src/states/state_stack.h \
    src/utils/resource_holder.h \
    src/utils/resource_identifiers.h \
    src/world.h

SOURCES += \
    src/SFX/music_player.cpp \
    src/SFX/sound_player.cpp \
    src/application.cpp \
    src/entity/debris.cpp \
    src/entity/dummy.cpp \
    src/entity/missile.cpp \
    src/entity/physics_object.cpp \
    src/entity/worm.cpp \
    src/main.cpp \
    src/player.cpp \
    src/states/game_state.cpp \
    src/states/state.cpp \
    src/states/state_stack.cpp \
    src/world.cpp
