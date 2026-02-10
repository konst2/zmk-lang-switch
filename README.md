[![ZMK](https://img.shields.io/badge/ZMK-Firmware-blue.svg)](https://zmk.dev)
[![MIT Licence](https://badges.frapsoft.com/os/mit/mit.png?v=103)](https://opensource.org/licenses/mit-license.php)


# zmk-lang-agnostic-behaviors

## Описание

Плагин для ZMK с кастомными `behaviors`, обеспечивающими независимость привязки символов от текущей раскладки ввода. Предназначен для клавиатур с двумя слоями раскладок: `английской` и `русской`. Можно использовать вместо русской какие-то другие, но это не проверялось.

Изначально вдохновлен плагином [zmk-lang-switch](https://github.com/xopclabs/zmk-lang-switch), однако, в результате, от этого плагина практически ничего не осталось.

Посмотреть пример его использования можно на моей [ZMK прошивке](https://github.com/konst2/sofle-custom-rus-zmk) для клавиатуры Sofle.

## Назначение

Решает проблему несоответствия физического расположения символов при переключении раскладок. Позволяет привязывать символы и модификаторы к клавишам так, чтобы их поведение оставалось предсказуемым независимо от активной раскладки ОС.


## Список behaviors

| Название | Параметры | Назначение |
|----------|-----------|------------|
| zmk,behavior-lang-switch | Слой | Переключение раскладки. Аргумент — целевой слой. Если текущий слой совпадает с аргументом — переключается на противоположную раскладку. |
| zmk,behavior-kp-on-lang | Символ | Нажатие символа только в заданной раскладке |
| zmk,behavior-modifier-press | Модификатор | Используется для того, чтобы нажатия клавиш вместе с модификаторами (Crtl, Alt, Cmd) осуществлялись на английской раскладке вне зависимости от текущей. |


## Требования

- разрабатывалось для ZMK v0.3.0
- только два слоя раскладки - `английский` и `русский`
- клавиши для переключения расладки должны быть разными для переключение на `английскую` и `русскую`

## Интеграция 

1. В файле `west.yml` прошивки (пример):
```yml
manifest:
  defaults:
    revision: v0.3
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    # Additional modules containing boards/shields/custom code can be listed here as well
    # See https://docs.zephyrproject.org/3.2.0/develop/west/manifest.html#projects
    - name: konst2
      url-base: https://github.com/konst2
  projects:
    - name: zmk
      revision: v0.3.0
      remote: zmkfirmware
      import: app/west.yml
    - name: zmk-lang-switch
      remote: konst2
      revision: main
  self:
    path: config
```
- в секции `remotes` добавляем
```yml
    - name: konst2
      url-base: https://github.com/konst2
```
- в секции `projects` добавляем
```yml
    - name: zmk-lang-switch
      remote: konst2
      revision: main
```

2. Определите идентификаторы слоёв раскладок в файле прошивки (например в `sofle.keymap`) и NEXT как константу 255

```c
#define ENGLISH 0
#define RUSSIAN 1
#define NEXT 255
```

3. В том же файле определяем behaviors (пример для модификации)
```
/ {
    behaviors {
        // переключение языка
        // аргумент -- номер слоя раскладки
        //             если аргумент не равен номеру слоя раскладки
        //             переключение производится на другую раскладку (не равную текущей)
        ls: lang_switch {
            compatible = "zmk,behavior-lang-switch";
            #binding-cells = <1>;
            // Коды клавиш  F17-переключение на EN F18 = переключение на RU
            bindings = <&kp F17>, <&kp F18>;
            en_layer = <ENGLISH>;
            ru_layer = <RUSSIAN>;
        };

        // нажатие символа принудительно в английской раскладке
        en: kp_on_eng {
            compatible = "zmk,behavior-kp-on-lang";
            #binding-cells = <1>;
            // Коды клавиш  F17-переключение на EN F18 = переключение на RU
            bindings = <&kp F17>, <&kp F18>;
            en_layer = <ENGLISH>;
            ru_layer = <RUSSIAN>;
            language = <ENGLISH>;
        };

        // нажатие символа принудительно в русской раскладке
        ru: kp_on_rus {
            compatible = "zmk,behavior-kp-on-lang";
            #binding-cells = <1>;
            // Коды клавиш  F17-переключение на EN F18 = переключение на RU
            bindings = <&kp F17>, <&kp F18>;
            en_layer = <ENGLISH>;
            ru_layer = <RUSSIAN>;
            language = <RUSSIAN>;
        };

        // использовать вместо &kp для модификаторов которые при удерживании должны работать только с английской раскладкой
        km: mod_press_with_counter {
            compatible = "zmk,behavior-modifier-press";
            #binding-cells = <1>;
            // Коды клавиш  F17-переключение на EN F18 = переключение на RU
            bindings = <&kp F17>, <&kp F18>;
            en_layer = <ENGLISH>;
            ru_layer = <RUSSIAN>;
        };
    };
};

```

по сути просто надо заменить клавиши которые используются для переключения на английскую и русскую раскладки.  
У меня:  
`F17` переключает на `En` раскладку;  
`F18` переключает на `Ru` раскладку.



## Использование в раскладке

Переключение раскладки
```
&ls NEXT  // NEXT=255 — всегда переключает на противоположную раскладку
&ls ENGLISH  // на En раскладку
&ls RUSSIAN  // на Ru раскладку
```

Символы вне зависимости от раскладки:
```
&en HASH   // Всегда вводит '#', даже если активна русская раскладка
&ru HASH   // Всегда вводит '№', даже если активна английская раскладка
```

Модификаторы (`Alt`, `Ctrl`, `Cmd`) с автоматическим переключением:
```
&km LCTRL  // При удержании Ctrl раскладка и слой всегда будут En
```

## Ограничения
- Поддерживает только две раскладки: английскую и русскую
- Требует фиксированных кодов переключения на разные раскладки (у меня `F17`/`F18`)

## Лицензия

MIT — совместима с лицензией основного проекта ZMK.
