# Worms SFML/C++
Worms is a classic game where several teams of worms use a variety of weaponry to elimiate each other from a randomly generated terrain. It based on this [repository](https://github.com/OneLoneCoder/Javidx9/blob/master/ConsoleGameEngine/BiggerProjects/Worms/OneLoneCoder_Worms3.cpp)

## Screenshots
![Image](https://user-images.githubusercontent.com/28188300/228937171-3cc4509d-db1d-4be5-818a-a431f4bf4410.gif)

### Installing
A step by step series  that tell you how to get a execute project.

Get it from GitHub
```
git clone git@github.com:Przemekkkth/WormsSFML.git
```

1) Using Qt

Compile
```
qmake && make
```

2) Using CMake

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```
```
cd build/
```
```
make -j4
```
```
cp -r ../res/ .
```
You can copy res directory using other method ie by GUI. Next, you can play it.

### Control

|     Key       | Action        |
| ------------- | ------------- |
| A             | decrease shot angle  |
| S             | increase shot angle  |
| Z             | jump  |
| SPACE         | make fire  |
| M             | mute/unmute  |
| Enter         | action  |

## Addons
* [SFML](https://www.sfml-dev.org/) - page of SFML project
* [Github](https://github.com/OneLoneCoder) - Github page of author
* [Music Theme](https://opengameart.org/content/title-theme-8-bit-style) - opengameart.org
* [Sound effects](https://opengameart.org/content/kelvin-shadewings-sound-pack-2) - opengameart.org 
* [Font](https://www.dafont.com/craftron-gaming.d6128) - page of author on dafont.com
* [yt](https://youtu.be/zZDiSESyBeg) - gameplay
