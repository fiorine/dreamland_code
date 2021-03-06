Есть несколько функций и методов для организации вывода сообщений игрокам. Они объявлены в плагине [output](https://github.com/dreamland-mud/dreamland_code/tree/master/plug-ins/output).
Здесь вкратце описывается их формат.

**Оглавление:**

* [Функция act_p](#act_p)
* [Функция fmt](#fmt)
* [Метод send_to у класса Character](#send_to)
* [Метод println у класса Character](#println)
* [Метод printf у класса Character](#printf)
* [Методы *echo у класса Character](#echo)
* [Функция page_to_char](#page_to_char)

## <a name="act_p">Функция act_p</a>
Это классическая функция, встречается в том или ином виде во многих движках-потомках ROM.
#### Формат: 
```c++
void act_p( const char *format, Character *ch, const void *arg1, const void *arg2, int type, int min_pos )
```
#### Аргументы:

**type**: Один из флагов, указывающий на то, кому пойдет сообщение. 
```
TO_CHAR    - увидит только char (ch). ch обязателен.
             act("Ты даешь $o4 $C4.", ch, obj, vch, TO_CHAR);
TO_VICT    - увидит только vch (другой персонаж, переданый как arg2).
TO_ROOM    - увидят все в комнате, кроме ch. ch должен указывать на того, кто делает act.
TO_NOTVICT - увидят все в комнате, кроме ch и vch. 
TO_ALL     - увидят все игрокам в комнате, включая ch и vch. 
```
**min_pos**: Один из флагов, указывающий на минимальную позицию с которой выдается данный act.
```
  POS_DEAD      0
  POS_MORTAL    1
  POS_INCAP     2
  POS_STUNNED   3
  POS_SLEEPING  4
  POS_RESTING   5
  POS_SITTING   6
  POS_FIGHTING  7
  POS_STANDING  8
```
В строке можно использовать следующие служебные символы, начинающиеся с $:
```
$e - местоимение "он/она"   в зависимости от пола ch
$E - местоимение "он/она"   в зависимости от пола vch
$s - местоимение "его/ее"   в зависимости от пола ch
$S - местоимение "его/ее"   в зависимости от пола vch
$m - местоимение "ему/ей"   в зависимости от пола ch
$M - местоимение "ему/ей"   в зависимости от пола vch
$x - местоимение "него/нее" в зависимости от пола ch
$X - местоимение "него/нее" в зависимости от пола vch
$y - местоимение "ним/ней"  в зависимости от пола ch
$Y - местоимение "ним/ней"  в зависимости от пола vch
$z - местоимение "нем/ней"  в зависимости от пола ch
$Z - местоимение "нем/ней"  в зависимости от пола vch
```
```
$g - указывает, что слово нужно разобрать в соответствии с полом ch.
     пример - $gприш|ело|л|ла 
$G - указывает, что слово нужно разобрать в соответствии с полом vch.
```
```
$c<n> - например, $c1. Подставляет имя ch в нужном падеже.
        где n 1 именительный (кто,что?) 
              2 родительный (кого,чего?)
              3 дательный (кому,чему?)
              4 винительный (кого,что?)
              5 тварительный (кем,чем?)
              6 предложный (о ком,о чем?)

$C<n> - Подставляет имя vch в нужном падеже.
$o<n> - подставляет название arg1 в нужном падеже
$O<n> - подставляет название arg2 в нужном падеже

$t - arg1 рассматривается как текстовая строка
$T - arg2 рассматривается как текстовая строка

$d  - подставляет название двери или "дверь"
```
#### Примеры
Рассмотрим на примере таких сообщений: 
1. Воздействия заклинаний на вещи, выводится всем в комнате:
```c++
   act("$o1 ярко вспыхивает и сгорает!", ch, obj, 0, TO_ALL).
```
   Все в комнате увидят, например:
```
   Свиток пергамента ярко вспыхивает и сгорает!
```

2. Социалы и эмоции: выводится персонажу и всем остальным в комнате.
```c++
   act("$c1 тихо хихикает.", ch, 0, 0, TO_ROOM);
   act("Ты тихо хихикаешь.", ch, 0, 0, TO_CHAR);
```
3. Взаимодействие между двумя персонажами: разные сообщения будут выданы актору, жертве и всем остальным.
Пусть ch - это персонаж Leo, vch - NoFate, obj - предмет меч.
```c++
   act("Ты даешь $o4 $C3.", ch, obj, vch, TO_CHAR);
   act("$c1 дает тебе $o4.", ch, obj, vch, TO_VICT);
   act("$c1 дает $o4 $C3.", ch, obj, vch, TO_NOTVICT);
```
Это превратится в:
```
Leo увидит:
   Ты даешь меч NoFate.
NoFate увидит:
   Leo дает тебе меч.
Все остальные увидят:
   Leo дает меч NoFate.
```
   
## <a name="fmt">Функция fmt</a>
Это более гибкая функция, позволяющая передавать позиционные параметры и форматировать вывод по ширине (как printf). Она не производит непосредственный вывод персонажу, а форматирует строку и возвращает ее. Формат:
```c++
DLString fmt(Character *to, const char *fmt, ...);
```
#### Аргументы

**to**: персонаж, от чьего имеи (чьими глазами) производится форматирование. Может быть NULL, тогда форматирование производится независимо от чьей-либо точки зрения.
        
**fmt**: строка в особом формате

**...**: произвольное количество аргументов

#### Символы внутри строки формата

Все спецсимволы начинаются с процента %, как и в printf:

```
%d, %i, %l       : целое число
%u               : беззнаковое целое число
%f               : число с плавающей точкой
%s               : строка
%S               : первое слово в строке
%c               : символ
%C1, %C2 ... %C6 : имя персонажа в указанном падеже, как его видит персонаж to
%O1, %O2 ... %O6 : название предмета в указанном падеже, как его видит персонаж to
%N1, %N1 ... %N6 : строка с падежами (стрел|а|ы|е|у|ой|е), в указанном падеже
%Gсрд|муж|жен    : варианты окончаний в зависимости от грам.рода параметра-персонажа или предмета,
                   например удари%Gло|л|ла
%Iодин|два|пять  : варианты окончаний в зависимости от параметра-числа, 
                   например золот%Iая|ые|ых монет%I|ы|ы
%P1, %P2 ... %P6 : личное местоимение в указанном падеже в зависимости от грам. рода 
                   и числа параметра-персонажа или предмета (его, ей, них) 
%T1, %T2 ... %T6 : притяжательное местоимение в указанном падеже в зависимости от грам. рода
                   и числа параметра персонажа или предмета (твой, твоего, твоей)
%p1, %p2 ... %p6 : личное местоимение в указанном падеже в зависимости от грам. рода, 
                   переданного в параметры как число
%gсрд|муж|жен    : варианты окончаний в зависимости от грам. рода, переданого в параметры как число
%nедин|множ      : варианты окончаний в зависимости от грам. числа параметра-персонажа или объекта,
                   например разбива%nется|ются
```

#### Форматирование по ширине

Запись %20s означает, что строку надо дополнить пробелами до 20 символов, если она короче.
%-20s означает то же самое, строка при этом сдвигается влево.

#### Заглавные и строчные буквы

Символ **^** означает, что данный аргумент надо написать с большой буквы, **_** - с маленькой. Обычно встречается в начале, чтобы имя персонажа в начале сообщения всегда было с заглавной буквы, например:
```c++
   fmt(ch, "%^C1 просыпается и садится отдыхать.", fido);
   "Ужасная зверюга просыпается и садится отдыхать."
   fmt(ch, "%C1 просыпается и садится отдыхать.", fido);
   "ужасная зверюга просыпается и садится отдыхать."
```

#### Позиционные параметры

Символы 1$, 2$ и так далее сразу после % указывают, к какому по счету аргументу относится данный формат.
Это проще объяснить на примерах, так
```c++
   fmt(to, "Помогите! На меня из засады напа%1$Gло|л|ла %1$C1!", killer);
```
аналогично записи
```c++
   fmt(to, "Помогите! На меня из засады напа%Gло|л|ла %C1!", killer, killer);
```
с разницей в том, что параметр killer достаточно указать только один раз, а потом ссылаться на него по номеру.

#### Примеры
```c++
fmt(0, "%2$^C1 отравле%2$Gно|н|на ядом от %3$O2 %1$C2.",  ch, victim, weapon);
"Злобный волколак отравлен ядом от кинжала Григория."
```
```c++
fmt(0, "У тебя %1$d практи%1$Iка|ки|к и %2$d тренировочн%2$Iая|ые|ых сесси%2$Iя|и|й.", prac, train);
"У тебя 18 практик и 2 тренировочные сессии."
```
```c++
fmt(0, "%^C1 стукается лбом о %N4.", ch, doorname);
"Руфина стукается лбом о дверь."
```

## <a name="send_to">Метод send_to у класса Character</a>

Метод ```send_to``` используется для простой отправки текста персонажу. Для удобства есть три варианта этого метода, принимающие в параметры строковую константу, ссылку на ```DLString``` или же на буфер ```ostringstream```.
Перевод строкки надо добавлять самостоятельно, или же использовать метод ```println``` (см. ниже). 

Примеры:
```c++
    ch->send_to("Ты не можешь избавиться от этого.\r\n");
```
```c++
    ch->send_to(fmt(0, "У тебя %1$d очк%1$Iо|а|ов опыта.\r\n", ch->exp.getValue());
```
```c++
    ostringstream buf;
    buf << "Твой уровень опытности: " <<  selfrate << "." << endl;
    ch->send_to(buf);
```

## <a name="println">Метод println у класса Character</a>

Аналогичен ```send_to```, но сам добавит в конце перевод строки:
```c++
    ch->println("У тебя нет такого свитка.");
```

## <a name="printf">Метод printf у класса Character</a>

Метод для формирования и вывода строки. Не самая удачная реализация, т.к. за кулисами вызывается обычный ```printf```, а не ```dlprintf```, который учитывает цвета при форматировании по ширине.

Пример:
```c++
ch->printf("Персонажу %s установлен основной дом в комнате [%d] %s.\r\n",
            pci->getName().c_str(), vnum, target->name );
```

## <a name="echo">Методы *echo у класса Character</a>

Это методы, использующие формат функции ```fmt``` и позволяющие вывести сообщение только персонажу, всем в комнате, кроме него, либо всем, кроме персонажа и жертвы. Главные метод в этом наборе - ```echo```, остальные методы в конечном итоге вызывают его же и добавлены для краткости.

### void Character::echo(int position, int type, Character *victim, const char *format, ...)

Параметры: 
* ```position``` - аналогичен параметру ```min_pos``` у функции [act_p](#act_p).
* ```type``` - аналогичен параметру ```type``` у функции [act_p](#act_p).
* ```format``` - формат как в функции [fmt](#fmt).
* ```victim``` - персонаж-жертва, используется для типов TO_VICT и TO_NOTVICT.

Примеры:
```c++
ch->echo(POS_RESTING, TO_NOTVICT, vict, "Эту строку увидят все неспящие, кроме меня и %C2.", vict);
ch->echo(POS_RESTING, TO_ROOM, NULL, "А эту увидят все, кроме меня.");
ch->echo(POS_RESTING, TO_CHAR, NULL, "Это сообщение увижу только я.");
```

### void Character::pecho( const char *f, ... ) и void Character::pecho( int pos, const char *f, ... )

Вывод только этому персонажу, возможно начиная с указанного "положения тела", по умолчанию - для всех положений.
Примеры:

```c++
client->pecho( "Твой кошелек стал легче на %1$d золот%1$Iую|ые|ых моне%1$Iту|ты|т.", cost );
ch->pecho( "%1$^O1 плотно закупоре%1$Gно|н|на пробкой, поищи штопор.", obj );
```

### void Character::recho( Character *vch, const char *f, ... )

Вывод всем в комнате, кроме этого персонажа и vch. Аналогичен вызову ```echo(POS_RESTING, TO_NOTVICT, vch, f)```.

### void Character::recho( const char *f, ... )

Вывод всем в комнате, кроме этого персонажа. Аналогичен вызову ```echo(POS_RESTING, TO_ROOM, 0, f)```.

### void Character::recho( int pos, const char *f, ... )

Вывод всем в комнате, начиная с положения тела pos, кроме этого персонажа. Аналогичен вызову ```echo(pos, TO_ROOM, 0, f)```.

## <a name="page_to_char">Функция page_to_char</a>

Эта функция используется для постраничного вывода. Если с помощью команды ```scroll``` задано ненулевое число строк, которые выводятся одновременно (по умолчанию 80), очень длинный текст будет выводиться постранично. После каждой страницы появится приглашение ```Нажмите Return для продолжения```.

Пример:
```c++
    // Сгенерировать статью справки для этого персонажа и отпечатать постранично:
    page_to_char(help->getText(ch).c_str(), ch);
```
```c++
    // Вывести в буфер заголовок и весь список зон, отпечатать постранично:
    ostringstream buf;
    buf << "{YВсе арии мира Dream Land: {x" << endl;
    // ...
    page_to_char(buf.str().c_str(), ch);
```
