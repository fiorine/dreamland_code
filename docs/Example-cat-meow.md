# Пример: мяукающий кот

## Задача

В доме живет кот, и нам хочется, чтобы кот периодически мяукал и драл мебель.

## Подход к решению

### 1. Выбор триггера и пример простого сценария

В [списке триггеров для мобов](https://github.com/dreamland-mud/dreamland_code/wiki/Fenia-char-triggers) мы видим два триггера, ```onSpec``` и ```onArea```, которые выполняются с какой-то периодичносью. Для нашей задачи больше подойдет ```onSpec```, т.к. в этом случае в действиях кота будет какой-то элемент неожиданности. ```onArea```, вызываемый раз в минуту, будет совпадать с другими действиями - сменой дня и ночи, спаданием аффектов, и смотреться не так красиво.

Триггер ```onSpec``` мы повесим на прототип котенка с внумом 3090 - он есть и в локальной версии.
Все экземпляры котенка, созданные, например, с помощью команды ```load mob 3090```, также будут иметь этот триггер. Все триггеры, висящие на прототипах, всегда первым параметром имеют конкретный экземпляр моба. В нашем примере этот параметр называется ```mob``` и содержит, соответственно, котенка, от которого сейчас вызвался тригер ```onSpec```.

С помощью [веб-редактора](https://github.com/dreamland-mud/dreamland_code/wiki/Fenia-editor) вобъем такой сценарий:
```
cs web
```
```javascript
catInit = function() {
    .get_mob_index(3090).onSpec = function(mob) {
        mob.interpret("эмоц громко и протяжно мяукает.");
    };
}
```
```
eval catInit()
load mob 3090
см

Черный Квадрат 
 Ты стоишь  на черном  игровом квадрате.  Ты видишь заброшенные ворота
недалеко на юге.

[Выходы: север юг запад]
Маленький верный котенок (kitten) здесь.

Котенок громко и протяжно мяукает.
```

### 2. Выполнение действия с какой-то вероятностью

Однако быстро становится понятно, что этот котенок будет не переставая мяукать каждые 4 секунды, что невыносимо.
Сделаем это мяуканье более случайным. Для работы со случайными числами есть несколько функций в корневом объекте.
Посмотрим по ним подсказку:
```javascript
eval ptc(.api())
...
chanceOneOf: (x) true если .number_range(1, x) == 1
chance: (x) true если x < .number_percent()
number_percent: произвольное число от 1 до 100
number_range: (x, y) произвольное число в промежутке от x до y
dice: (x, y) x раз кинуть кубик с y гранями
```

Нам удобен метод ```.chance```: так, ```.chance(50)``` вернет ```true``` в 50ти случаях из ста, т.е. с вероятностью 50%, ```.chance(1)``` вернет ```true``` с вероятностью 1% и так далее.

Перепишем сценарий:
```javascript
catInit = function() {
    .get_mob_index(3090).onSpec = function(mob) {
        if (.chance(20))
            mob.interpret("эмоц громко и протяжно мяукает.");
    };
}
```

Теперь он мяукает не так часто, но достаточно часто для отладки. В окончательной версии шанс можно сделать равным 1.

### 3. Выполнение нескольких действий с равной вероятностью

Предположим, мы хотим добавить ему еще какие-то действия с равной вероятностью. Для этого пригодится метод ```.chanceOneOf```. Вот как будет выглядеть новый сценарий с тремя случайными действиями котенка:
```javascript
catInit = function() {
    .get_mob_index(3090).onSpec = function(mob) {
        if (.chance(20)) {
            if (.chanceOneOf(3)) 
                mob.interpret("эмоц громко и протяжно мяукает.");
            else if (.chanceOneOf(2))
                mob.interpret("эмоц зевает и грациозно потягивается.");
            else
                mob.interpret("эмоц гоняется за собственным хвостом.");
        }
    };
}
```

### 4. Поиск предмета и действия с ним

Следующий шаг - сделать, чтобы котенок драл мебель. Для начала выясним, есть ли рядом с ним в комнате мебель,
т.е. предмет, лежащий на полу, и имеющий тип ```furniture```. Все типы предметов описаны в таблице ```.tables.item_table```, содержимое таблицы можно вывести на экран:
```javascript
eval ptc(.tables.item_table.api())
```
Список всех доступных таблиц с флагами:
```javascript
eval ptc(.tables.api())
```

Первая колонка содержит символическое название типа предмета, оно же используется в зонах и редакторе OLC. Для мебели это поле ```.tables.item_table.furniture```.

Теперь надо найти в комнате предмет-мебель, используя метод комнаты ```get_obj_type```. Вот его справка в API комнаты:
```javascript
eval ptc(in_room.api())
...
get_obj_type: (type) поиск объекта в комнате по его типу, item type
```
Для отладки создадим рядом с котенком какую-то мебель (на локальном мире можно поискать мебель командой ```vnum type furniture```).
```javascript
load obj 9822
Ты создаешь диван!
```
А затем найдем эту мебель рядом с нами в комнате и отпечатаем ее имя:
```javascript
eval sofa=in_room.get_obj_type(.tables.item_table.furniture)
eval ptc(sofa.name)
sofa диван
```

Для того, чтобы вывести сообщение о действии, выполняемом с предметом, воспользуемся методом персонажа ```recho``` - вывести строку всем в комнате. Подробнее о формате сообщений смотреть [в этой статье](https://github.com/dreamland-mud/dreamland_code/wiki/ACT-format#%D0%A4%D1%83%D0%BD%D0%BA%D1%86%D0%B8%D1%8F-fmt). Статья описывает функцию ```fmt```, используемую в коде, однако в фене формат сообщений точно такой же.

## Готовый сценарий

Вот как теперь выглядит сценарий. Дабы избежать вытягивания кода в "спагетти", сделаны некоторые изменения:

```javascript
catInit = function() {
    .get_mob_index(3090).onSpec = function(mob) {
        if (!.chance(1))
            return;

        if (.chanceOneOf(4)) 
            mob.interpret("эмоц громко и протяжно мяукает.");
        else if (.chanceOneOf(3))
            mob.interpret("эмоц зевает и грациозно потягивается.");
        else if (.chanceOneOf(2))
            mob.interpret("эмоц гоняется за собственным хвостом.");
        else {
            var furniture;
                
            furniture = mob.in_room.get_obj_type(.tables.item_table.furniture);
            if (furniture == null)
                return;
            
            mob.recho("%^C1 дерет когтями %O4.", mob, furniture);
        }
    };
}
```








 