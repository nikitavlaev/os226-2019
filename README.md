# os226-2019

## Как сдавать

Задания предлагается сдавать в виде Pull Requestов.

Язык общения в PR/issues: русский

На каждый PR travis запускает набор тестов.

Если тесты успешно проходят на travise и в коде нет криминала, PR получает метку "accepted" и закрываются.
PR с меткой accepted учитываются на зачёте в положительную сторону.

Если тесты не проходят, то PR получает метку "issues" и вам нужно исправлять код.

Чтобы проверить тесты локально (до коммита/пуша/создания PR), достаточно запустить `test/run.sh` в директории текущего упражнения.
Например:
```
os226-2019/example$ ./test/run.sh ; echo $?
```
echo выводит код возврата, он должен быть нулём.

Если тесты прошли, но в коде есть грубые ошибки, то PR получает метку "issues" и вам нужно исправлять код.
Пример грубой ошибки: модификация тествой системы, чтобы она засчитывала неверные решения :-)

Для того, чтобы изменить уже посланое решение, достаточно сделать push в ваш бранч.
PR автоматически обновится и перетестируется.
