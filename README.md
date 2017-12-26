```
           _                  _
  ___ ___ (_)_ __   __ _  ___| |_
 / __/ _ \| | '_ \ / _` |/ _ \ __|
| (_| (_) | | | | | (_| |  __/ |_
 \___\___/|_|_| |_|\__, |\___|\__|
                   |___/
```

Display the values and ranking of your favorite crypto currency in your terminal.💸

## Help ##

      -c, --convert=SYM          display value in currency
      -i, --coin id=SYM          display specific crypto
      -l, --limit=NUM            display NUM cryptos
      -s, --start=NUM            start displaying from given rank
      -?, --help                 Give this help list
          --usage                Give a short usage message
      -V, --version              Print program version

## Bugs ##
Report bugs or questions to [https://github.com/Olavhaasie/coinget/issues/new].

## Dependencies ##
+ [https://curl.haxx.se/libcurl/](libcurl)
+ [https://www.gnu.org/software/libc/manual/html_node/Argp.html](argp)
+ and of course [https://github.com/zserge/jsmn](jsmn)

The first two must be downloaded/installed. The JSON library is included as git sub module. The code was developed on Linux, but will also possibly work on MacOS.

## Compile ##
first clone the repository

    git clone --recursive https://github.com/Olavhaasie/coinget

The option `recursive` will also clone the `jsmn` dependency. Then

    make

And now you can run with

    ./coinget

## License ##
This software is distributed under MIT license 📝

