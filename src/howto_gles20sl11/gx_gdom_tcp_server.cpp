#include <gx_gdom_tcp_server.h>

#define GX_NODE_INFO_FAVICON_ICO \
    "<link href=\"data:image/x-icon;base64,AAABAAEAEBAQAAAAAAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAg"\
    "AAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAEhEQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"\
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"\
    "AAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD"\
    "//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP7/AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA\""\
    " rel=\"icon\" type=\"image/x-icon\" />"


#define GX_NODE_INFO_HTML_STYLE \
    "<style>"\
    ".body_style {"\
        "background-color:black;"\
    "}"\
    ".queryHead {"\
        "border-top-right-radius: 6pt;"\
        "border-top-left-radius: 6pt;"\
        "border-bottom: none;"              /* Убираем линию снизу */\
        "padding: 3px 7px 2px 7px;"         /* Поля вокруг текста */\
        "display: inline;"                  /* Устанавливаем как встроенный элемент */\
        "background: #efecdf;"              /* Цвет фона */\
        "font-family: monospace;"           /* Шрифт текста */\
        "font-weight: bold;"\
        "font-size: 90%;"\
        "margin: 0; "                       /* Убираем отступы вокруг */\
        "white-space: nowrap;"              /* Отменяем переносы текста */\
    "}"\
    ".queryJson {"\
        "border-top-right-radius: 3pt;"\
        "border-bottom-left-radius: 3pt;"\
        "border-bottom-right-radius: 3pt;"\
        "background: #bbc6d4; "     /* Цвет фона */\
        "font-family: monospace;"   /* Шрифт текста */\
        "padding: 3 0 0 7;"         /* Поля вокруг текста */\
        "margin: 1 0 0 0;"          /* Отступы вокруг */\
        "white-space: pre;"         /* Учитывать переносы и пробелы */\
    "}"\
    ".dict {"\
        "display:block;"\
        "color:#e2e2e2;"\
        "font-family:monospace;"\
        "text-decoration-line:none;"\
        "padding:0 0 0 6pt;"\
        "margin:0;"\
        "white-space:pre;"\
    "}"\
    ".path {"\
        "border-radius:2pt;"\
        "background-color: rgba(117, 117, 117, 0.56);"\
        "color: #e2e2e2;"          /* Цвет фона */\
        "font-family: monospace;"   /* Шрифт текста */\
        "padding: 0 0 0 1pt;"         /* Поля вокруг текста */\
        "margin:  0;"        /* Отступы вокруг */\
        "white-space: pre;"         /* Учитывать переносы и пробелы */\
    "}" \
    ".menu {"\
        "color: #aaa; "\
        "font-family: monospace;"\
        "padding: 0 0 0 6pt;"\
        "margin:  0;"\
        "white-space: pre;"\
    "}" \
    ".request {"\
        "border: 1px solid black;"  /* Параметры рамки */\
        "background: #3f3f46; "     /* Цвет фона */\
        "color: #fff; "             /* Цвет фона */\
        "font-family: monospace;"   /* Шрифт текста */\
        "padding: 3 0 0 7;"         /* Поля вокруг текста */\
        "margin: -1 0 0 0;"        /* Отступы вокруг */\
        "white-space: pre;"         /* Учитывать переносы и пробелы */\
    "}"\
    ".inspect {"\
        "border: 1px solid black;"  /* Параметры рамки */\
        "background: #634e4e; "     /* Цвет фона */\
        "color: #efefe0; "             /* Цвет фона */\
        "font-family: monospace;"   /* Шрифт текста */\
        "padding: 3 0 0 7;"         /* Поля вокруг текста */\
        "margin: -1 0 0 0;"        /* Отступы вокруг */\
        "white-space: pre;"         /* Учитывать переносы и пробелы */\
    "}"\
    ".error {"\
        "color: #ff0; "             /* Цвет фона */\
        "font-family: monospace;"   /* Шрифт текста */\
        "padding: 0 0 0 6pt;"       /* Поля вокруг текста */\
        "margin: 0;"                /* Отступы вокруг */\
        "white-space: pre;"         /* Учитывать переносы и пробелы */\
    "}"\
    "</style>"


/**  ОПРЕДЕЛЁН ТИП КЛИЕНТА, ЭТО БРАУЗЕР ИЛИ ЕГО ИММИТАЦИЯ

 Тип этого запроса не задан шаблоном "HTTP_GET_REQUEST", - в данном случае тип назначен принудительно,
 но в общем случае тип берется из шаблона, и используется как имя фабрики. Должна быть фабрика по и для
 производства GDOM пути из gx::json* ...

 На самом деле это обозначает, что имя шаблона производит(генерирует) инстанцию прототипа, с полностью
 законченными полями (заполнено ссылками на их прототипы). Визитирование запроса и этого шаблона одновременно
 - суть процесса порождающего инстанцию выверенного (проконтролированного по типам и размерам) ПРЕКОМПИЛЕННОГО
 ЗАПРОСА. Проблема в том, что проверки типов и подтипов, многочисленные сравнения и вызовы NEW - работа для
 нити(thread). А вот внедрение полностью сформированной(прекомпиленной) схемы транзакции - это работа для
 главного процесса приложения - (СТРОГО ДЕТЕРМИНАНТНО БЕЗ АСИНХРОНА). При "внедрении" важна скорость и
 минимизация лага. Потому как в главном(ведущем) процессе вертится MAIN EVENTS LOOP.

 Ошибка которую просто совершить при реализации: поставить прототип во главу иерархии приложения. Нельзя этого
 делать по простой причине. Писатель (getter) и читатель (setter) в отличие от строителя (builder) при проверке
 типов сырого JSON руководствуются путями "живого" GDOM, набор путей которые нужно записать или прочитать за одну
 транзакцию. А builder - читает вечный и неразрушимый прототип (всегда в асинхроне) и строит набор инстанций
 (иногда и одну) в теле инстанций GETTER|SETTER. Которые и осуществляют доставку предварительно достроенных
 инстанций в ЖИВОЙ GDOM для записи/чтения "ОДНОЙ ТРАНЗАКЦИЕЙ".

                Ближайшая аналогия: Хирург мировое светило с зарплатой $10000+ в час, заходит на операцию
                в полностью подготовленное помещение. Больной на столе, все инструменты, ассисстенты, все
                приборы на ходу и включены. Работает 15 минут. Бросает завершающие операции на ассистентов
                и идет к следующей готовой операционной. Ни разу не ждет, пока что либо доставят. Линия
                действий светила - аналог главного "main-thread", master. А линии действий всего остального
                персонала не исключают ожиданий, проверок или даже отказов от проведения операции. Задолго
                до приглашения светила к работе. - Это асинхронные процессы, даже множество параллельных
                процессов, по подготовке окружения к транзакции "tool-thread". Результат операций не всегда
                успех, но доставку результата в морг и ли в палату для выздоровления опять же осуществляют
                эти slave процессы. Иначе говоря: процесс не может одновременно быть и быстрым и ждущим.

                Это main-thread, разворачивать проверки на интерфейс - значит порождать lag в gui.
                но мы предполагаем чтение определенного пути, а если чертов путь удалится, то и
                читающий асинхронный процесс навернётся, с непредсказуемыми последствиями. Можно
                увеличить ссылку на каждый элемент (словарь) вдоль пути, а по завершении снять их
                все, но это нарушит ход удаления "мёртвых" объектов из "gdom". Нам просто известно
                что "HTTP_GET_REQUEST" - отладочный "view" "gdom"-модели. Который не должен влиять
                на время жизни просматриваемого пути. Это значит, что весь путь и все операции в нем
                нужно выполнить синхронно в "main-thread". Нечего вынести в асинхронный процесс. Для
                полноты аналогии, это случай, когда заменить "светило" в деле отправления малой нужды
                толпы ассистентов не смогут помочь, а будут только мешать.

                Налицо необходимость двойных стандартов при подходе к программированию каждого автомата
                в GDOM. С одной стороны это интерфейс предусматривающий обращение через 'front door'. Это
                официальный "интерфейс" предусматривающий объекты "GETTER"/"SETTER", которые увеличивают
                'hardref' запрошенного узла, инициируют смену состояний узла, и ЖДУТ состояния автомата
                пригодного для выполнения своей задачи - чтения, записи, или непрерывного оповещения об
                изменении значения или состояния. С другой стороны, это подглядывание через 'back door'.
                Подглядывание не изменяет значений и состояний узла, не увеличивает числа ссылок, хотя и
                позволяет сфотографировать текущие значения и состояния. Это позволяет "ИНСПЕКТИРОВАТЬ"
                узел не изменяя его. Т.е. модифицировать поведение кода визитирующего модель, без того,
                чтобы модифицировать состояние модели. А это чистый "MVP" или "MVС".
**/

struct gdom_try_json_path
{
    gx::gdom* mp_root;  // initial gdom node
    gx::gdom* mp_curr;  // current gdom node
    gx::gdom* mp_gdom;  // target gdom node => NULL if error found
    gx::json::LIST* mp_path;  //
    std::ostringstream oss;   // error info
    unsigned int current_segment_number;
    gdom_try_json_path( gx::json* p_path, gx::dict* p_from = gx::gdom::root() )
        : mp_root(p_from)
        , mp_curr(p_from)
        , mp_gdom(NULL)
        , current_segment_number(0)
    {
        if ( ! p_path->is_list() )
        {
            oss << "path type is not some json::LIST";
            return;
        }

        mp_path = (gx::json::LIST*) p_path;

        gx::json::LIST::iterator segm = mp_path->begin();
        gx::json::LIST::iterator stop = mp_path->end();
        for(; segm != stop; ++segm )
        {
            // CHECK PATH ITEM IS COMPATIPLE WITH JSON QUERY RULES
            struct: gx::json::proc { // extend abstract JSON visitor
                const char*                  type_name;
                void on( gx::json::DICT* ) { type_name = "DICT" ; }
                void on( gx::json::LIST* ) { type_name = "LIST" ; }
                void on( gx::json::NONE* ) { type_name = "NONE" ; }
                void on( gx::json::CSTR* ) { type_name =  NULL  ; /* "CSTR" ACCEPTED */ }
                void on( gx::json::SINT* ) { type_name = "INT"  ; }
                void on( gx::json::UINT* ) { type_name = "INT"  ; }
                void on( gx::json::REAL* ) { type_name = "REAL" ; }
                void on( gx::json::BOOL* ) { type_name = "BOOL" ; }
            }   unexpected; (*segm)->on(&unexpected);  // make-visitor-call-visitor

            if (unexpected.type_name)                  // use visitor's result
            {
                oss << "ERROR:Path["<< current_segment_number <<"] has unexpected type '"<< unexpected.type_name << "'";
                break;
            }

            // MAKE GDOM-DICT ACCEPTABLE KEY INSTANCE FROM PATH JSON KEY
            struct: gx::json::proc {   // extend abstract JSON visitor
                gx::i3vu key;          // make gdom 'dict' acceptable key
                void on(gx::json::DICT*  ) { key.data = "" ; }  // "" => key type error
                void on(gx::json::LIST*  ) { key.data = "" ; }  // "" => key type error
                void on(gx::json::NONE*  ) { key.data = "" ; }  // "" => key type error
                void on(gx::json::CSTR* o) { key.data = o->value; } // CSTR accepted
                void on(gx::json::SINT*  ) { key.data = "" ; }  // "" => key type error
                void on(gx::json::UINT*  ) { key.data = "" ; }  // "" => key type error
                void on(gx::json::REAL*  ) { key.data = "" ; }  // "" => key type error
                void on(gx::json::BOOL*  ) { key.data = "" ; }  // "" => key type error
            } gdom_ascii;  (*segm)->on(&gdom_ascii);            // make-visitor-call-visitor
            // KEY TYPE ARE GDOM 'DICT' COMPATIBLE, BUT TYPE OF THE CURRENT GDOM NODE
            // ARE "UNKNOWN". SO, CHECK KEY TYPE COMPATIBILITY WITH CURRENT GDOM NODE
            if( ! mp_curr->is_acceptable_query_type(&gdom_ascii.key) )
            {
                oss << "Path[" << current_segment_number << "] has unsupported type '" << mp_gdom->str_type_name() << "'";
                break;
            }

            // KEY TYPE ARE ACCEPTABLE, BUT RESULT OF THE QUERY THIS KEY - UNPREDICTABLE.
            // NULL-RESULT MEAN 'KEY VALUE ERROR'
            gx::gdom* p_next = mp_curr->query(&gdom_ascii.key);
            if (NULL == p_next)
            {
                oss << "Path[" << current_segment_number << "] key '" << gdom_ascii.key.data << "'" << " not exists";
                break;
            }

            // WE HAVE NON NULL RESULT, SO CONTINUE PATH DIGGING LOOP
            mp_curr = p_next;  // STORE CURRENT AS LAST VISITED GDOM NODE
            ++current_segment_number;
        }
        if ( current_segment_number == mp_path->size() )
        {
            mp_gdom = mp_curr;
        }
    }
};

void error(std::ostream& html, const std::string &text) {
    html << "<p class=\"error\"><b><i>" << text << "</b></i></p>";
}

void gx_gdom_tcp_reactor::on_query_is_http_get_request()
{
    // ЭТОТ МЕТОД ВЫПОЛНЯЕТ ОТВЕТ НА ЗАПРОС "HTTP_GET_REQUEST". В "htm" БУДЕТ СФОРМИРОВАН "HTML".
    // ЕСЛИ ЗАПРОС БУДЕТ СОДЕРЖАТЬ ОШИБКИ СИНТАКСИСА, КАК И ОШИБКИ СВЯЗАННЫЕ С СОСТОЯНИЕМ "GDOM",
    // ТО В РЕЗУЛЬТИРУЮЩИЙ "HTML" ПРОСТО ОТПРАВИТСЯ ИХ ПЕРЕЧЕНЬ. ПРИ РАБОТЕ С СЕРВЕРОМ ВОЗНИКАЕТ
    // ИЛЛЮЗИЯ ЧТО ЗАПРОСЫ НАПРАВЛЕНЫ В ФАЙЛОВУЮ СИСТЕМУ, НО ЭТО НЕ ТАК. ПРИ ОБРАЩЕНИИ К ФАЙЛАМ,
    // КАК И ПРИ ОБРАЩЕНИИ К КАТАЛОГАМ, ПРОИСХОДИТ АБСОЛЮТНО ДРУГОЕ. В ШИРОКО РАСПРОСТРАНЕННЫХ
    // СИСТЕМАХ, ОСНОВАНЫХ НА РАЗДАЧЕ ФАЙЛОВ, В КАТАЛОГЕ ПО ПУТИ ЗАПРОСА, НАХОДИТСЯ ФАЙЛ КОТОРЫЙ
    // ОПРЕДЕЛЯЕТ ЧТО ИМЕННО ВЕРНЁТСЯ ВМЕСТО ФАЙЛОВОГО СОДЕРЖИМОГО ПО ЭТОМУ ПУТИ URL С ТЕКУЩИМИ
    // АРГУМЕНТАМИ ЭТОГО URL. ЕСЛИ ПРЕДПОЛОЖИТЬ ЧТО ЭТО ПИТОНОВСКИЙ СКРИПТ, ШЕЛЛ СКРИПТ ИЛИ ДАЖЕ
    // ИСПОЛНЯЕМЫЙ ФАЙЛ, ОН ПРОСТО ВЫПОЛНЯЕТСЯ КАК ПРОЦЕСС ОПЕРАЦИОННОЙ СИСТЕМЫ С АРГУМЕНТАМИ ИЗ
    // ЭТОГО URL В КАЧЕСТВЕ АРГУМЕНТОВ КОМАНДНОЙ СТРОКИ. РЕЗУЛЬТАТ ВЫПОЛНЕНИЯ, ИЗ ПОТОКА ВЫВОДА
    // ЭТОГО ПРОЦЕССА, КОПИРУЕТСЯ В СОКЕТ ИСТОЧНИКА ЗАПРОСА. НЕ ГУСТО, НЕ ПУСТО, НО РЕЗУЛЬТАТОМ
    // ЭТОГО БЕСХИТРОСНОГО ПОДХОДА ЯВЛЯЕТСЯ ПОЧТИ ВЕСЬ СОВРЕМЕННЫЙ (год 2017) ВЕБ. ВО ИЗБЕЖАНИЕ
    // ЗЛОНАМЕРЕННОГО ВЗЛОМА ОПЕРАЦИОНКИ С ТАКИМ СЕРВЕРОМ ВСЕ, ПОТЕНЦИАЛЬНО ОПАСНЫЕ ПРОЦЕССЫ НЕ
    // СТАРТУЮТ НА САМОЙ ОПЕРАЦИОНКЕ, А ДЕЛЕГИРУЮТСЯ ДРУГОЙ - УДАЛЕННОЙ МАШИНЕ, ВИРТУАЛЬНОЙ ИЛИ
    // ЕЩЁ КАКОЙ ЛИБО МАШИНЕ, НА КОТОРОЙ ФОРМИРУЮТСЯ РЕЗУЛЬТАТЫ ЗАПРОСА, ВОЗВРАЩАЮТСЯ НА МАШИНУ
    // С КОТОРОЙ И БЫЛО УСТАНОВЛЕНО СОЕДИНЕНИЕ, И КОПИРУЕТСЯ В СОКЕТ ИСТОЧНИКА ЗАПРОСА. МАШИНЫ,
    // РАБОТА КОТОРЫХ ЗАКЛЮЧАЕТСЯ ТОЛЬКО В ТОМ, ЧТОБЫ ПЕРЕНАПРАВЛЯТЬ ЗАПРОСЫ, А ЗАТЕМ ОТДАВАТЬ
    // ПОЛУЧЕННЫЕ РЕЗУЛЬТАТЫ - РАУТЕР, ПРОКСИ СЕРВЕР, В КАКОМ-ТО СМЫСЛЕ И DNS СЕРВЕР - ПРОСТО
    // ЭЛЕМЕНТЫ МАРШРУТИЗАЦИИ ЗАПРОСОВ-ОТВЕТОВ. СОСТАВЛЯЮТ ИНФРАСТРУКТУРУ ВЕБ. ЯВЛЯЮТСЯ ОСНОВОЙ
    // ДЛЯ СИСТЕМ ОПТИМИЗАЦИИ ВЫПОЛНЕНИЯ ЗАПРОСОВ. ИЛИ РАСПРЕДЕЛЯЯ ЗАПРОСЫ МЕЖДУ ЭКВИВАЛЕНТНЫМИ
    // СЕРВЕРАМИ, ИЛИ ГРУППИРУЮЯ АБСОЛЮТНО ЭКВИВАЛЕНТНЫЕ ЗАПРОСЫ - ДЛЯ ВОЗВРАТА ИМ ВСЕМ ОДНОГО
    // РЕЗУЛЬТАТА, ТОЛЬКО ОДНОГО ЗАПРОСА, ИЛИ НАКАПЛИВАЮЩАЯ ПАРЫ ЗАПРОС/ОТВЕТ ДЛЯ ТОГО, ЧТОБЫ
    // ВООБЩЕ ИЗБЕЖАТЬ ДЕЛЕГИРОВАНИЯ ЗАПРОСА, СРАЗУ ВОЗВРАЩАЯ ОТВЕТ, ЕСЛИ ЕГО СРОК ГОДНОСТИ НЕ
    // ИСТЁК (REST). ДАННЫЙ СЕРВЕР НЕ ФАЙЛОВЫЙ, ИМЕЕТ СОВЕРШЕННО ДРУГУЮ ОРГАНИЗАЦИЮ, И ПРИЧИНУ
    // ВОЗНИКНОВЕНИЯ, НО ДОЛЖЕН ИСПОЛЬЗОВАТЬ УЖЕ СЛОЖИВШУЮСЯ ИНФРАСТРУКТУРУ ВЕБ. ЗАТОЧЕННУЮ ПОД
    // REST И СОСТОЯЩУЮ ИЗ МИЛЛИОНОВ ЕДИНИЦ КАБЕЛЕЙ, ОТДЕЛЬНЫХ МАШИН И ЦЕЛЫХ IT-КОМПАНИЙ.

    // ИТАК,
    // В ЧЕМ ОТЛИЧИЯ: ПУТЬ URL ЗАПРОСА, ЭТО ПУТЬ К ЭЛЕМЕНТУ МОДЕЛИ ДАННЫХ, КОТОРЫЙ РАСПОЛОЖЕН В
    // ПАМЯТИ ИНСТАНЦИИ УЖЕ СУЩЕСТВУЮЩЕГО ПРОЦЕССА. ВИДНА ЛИ ДАННОЙ ИНСТАНЦИИ ФАЙЛОВАЯ СИСТЕМА,
    // ЭТО ЕЩЁ ТОТ ВОПРОС. ДЛЯ HTML GET ЗАПРОСА, ЕСЛИ ПУТИ НЕ СУЩЕСТВУЕТ, ВОЗВРАЩАЕТСЯ СТРАНИЦА
    // С JSON ВЕРСИЕЙ GET ЗАПРОСА, ОПИСАНИЕМ ОШИБКИ, И СРЕДСТВОМ РУЧНОЙ НАВИГАЦИИ ПО ПУТЯМ. САМ
    // "НАВИГАТОР" УСТАНОВЛЕН В СОСТОЯНИЕ ПРОСМОТРА БЛИЖАЙШЕГО К ЗАПРОШЕННОМУ, НО СУЩЕСТВУЮЩЕГО
    // В СИСТЕМЕ ПУТИ. ЕСЛИ ПУТЬ, КАКИМ ЛИБО ОБРАЗОМ, ДОЛЖЕН ЭМУЛИРОВАТЬ ФАЙЛОВЫЕ ПУТИ, БАЗОВОЕ
    // ПРАВИЛО ПРИЛОЖЕНИЯ, ПРОВЕРЯТЬ НАЛИЧИЕ ТАКОГО ОТНОСИТЕЛЬНОГО ПУТИ, РАСПОЛАГАЕТСЯ ПО ПУТИ
    // "/file". Вот тут и должен сидеть реактор выхолащивающий систему до транспортирования байт
    // из файловой системы в веб. Но даже тут имеются отличия. Даже правильный путь вернёт сто,
    // или несколько сот первых байт файла. Если нужно больше, - повторите запрос с аргументами,
    // с какой позиции и сколько байт(не больше чем лимитировано в системе) нужно вернуть. Всё?
    // Нет не всё. Этой системе не чужд трюк с процессом, который можно запустить с аргументами
    // командной строки. Неужели вернулась фича цэгейи (COMMON GATEWAY INTERFACE). Нет. Совсем
    // другой прибамбас - это чудо запустит скрипт и получит JSON ответ, в котором будет путь к
    // продолжению запроса на этом-же сервере. Смысл в том, что пока не нужен был этот объект,
    // а это сохранёнка состояния какого-то бота в сети, то и сервис связанный с его активацией
    // не был нужен. Но если бот всё таки востребован, и доступ к этому боту в пределах прав
    // доступа для данной сессии связи, то возобновляется целый процесс его работы. Для того,
    // чтобы устроить это чудо, процесс, в качестве аргумента, получит ключ для поиска сессии
    // с сокетом ожидающим ответа. Запустит актора соответствующего типа, дождется его отклика,
    // передаст ему все полученные аргументы и завершится. Но вот сам инициализированный бот,
    // найдет сессию по ключу, и впишет туда ответ, на манер "Чё нада?".
    //
    // НАХРЕНАТОР: Ключевой алгоритм этого сервера, это обращение к инстанции, размещенной в
    // оперативной памяти по определенному пути. Обращение для проверки наличия, обращение для
    // отображения, обращение для выполнения SET или GET, обращение к дереву типов, для NEW --
    // всё это "обращения по пути". Сами пути разделяются по признаку, удерживают они инстанцию
    // полностью созданного объекта в памяти, или не удерживают.
    //
    // "Дебильный подход к NEW" : NEW обязано проверить есть ли по этому пути объект, затем
    // проверить есть ли сохранение этого объекта, загрузить его, объяснить ему ситуацию __DEL__
    // по этому пути, снести его сохраненки, если __DEL__ этого ещё не сделал, разместить
    // сохраненку пустого нового объекта указанного типа, загрузить его, разъяснить ситуацию
    // __NEW__. В общем геммор, если "создание" "нового" объекта не отделено от "размещения"
    // этого "нового" по определенному пути в системе.
    //
    // На самом деле, если объект имеет набор методов определяемый набором действий над ним в
    // системе, некий "SYSTEM_INTERFACE", то что он будет содержать?
    // __on_system_type_assigned__  // объект получил путь в пространстве типов (это как обретение себя :)))
    // __on_create_instance_start__  // этот метод вызывается до того, как стартует thread-строитель
    //                          это нужно для того, чтобы стартовать в параллель другие задачи и/или
    //                          распараллелить "строитель" между несколькими thread-ами, что может
    //                          существенно ускорить "строитель" включающий несколько сетевых операций
    // __on_create_instance_finish__  // этот метод вызывается когда thread-строитель завершен
    //   Если объект имеет собственное мнение о том, как себя вести в новом окружении:
    //    - Окружение, это то, что даёт каждый новый путь, а их у каждой инстанции (0..N)
    //   И каждый путь может как удерживать инстанцию в памяти, так и предъявлять к ней свои требования.
    // __on_system_path_connected__   // с точки зрения формальной логики, объект получил путь (это как обретение пути :)))
    // __on_system_path_removed__     // с точки зрения формальной логики, объект потерял путь
    //   Если инстанцию в памяти уже невозможно отыскать ни по какому пути, это не значит что её
    //   удаление завершено, это значит что пора писать завещание для собственной реинкарнации в
    //   в следующей жизни. Дело в том что не все объекты умеют сохраняться для реинкарнации.
    //   многим достаточно типа (достаточно быть собой). Но вот обретение файлового пути, это то
    //   что поможет инстанции, заменить своё значение по умолчанию на последнее сохраненное.
    //   С++ отравляет мозг разработчика иллюзией, что всем инстанциям насрать на свои контейнеры.
    //   Я есть int! - Мне насрать на мои контейнеры, массив ли это, словарь ли это, функция ли это - МНЕ ПОФИГ!
    //   Но вот проецировать эту сагу 'int'-а на инстанции других типов в объектно-ориентированном
    //   программировании - ЧИСТОЕ ЗЛО.

    // INSERT ICON - IS ONLY PREVENT SEPARETE GET FOR "favicon.ico" (DMFD).
    // or using "<link href=\"favicon.ico\" rel=\"icon\" type=\"image/x-icon\" />"

    std::ostringstream htm;

    // Это генерирует шапку HTML страницы для доставки результатов запроса
    /// TODO: void gx_gdom_tcp_reactor::html_gen_head(std::ostream& htm);
    //
    htm << "<html><head>";
    htm << "<title>" << "url.path().toStdString()" << "</title>";
    htm << GX_NODE_INFO_FAVICON_ICO;
    htm << GX_NODE_INFO_HTML_STYLE;
    htm << "</head><body class=\"body_style\">";

    // Это дамп запроса, как его понял конвертер из HTTP GET request в JSON "HTTP_GET_REQUEST"
    /// TODO: void gx_gdom_tcp_reactor::html_gen_request_echo(std::ostream& htm);
    //
    htm << "<p class=\"queryHead\">" << "Query as JSON" << "</p>";
    htm << "<p class=\"queryJson\">";
    htm << gx::json_text_dump(p_root);
    htm << "</p>";

    // Вот в этом месте можно заливать в "htm" ответ на запрос
    /// TODO: void gx_gdom_tcp_reactor::html_gen_responce request_echo(std::ostream& htm);
    //
    do {
        if( ! p_root )            { error( htm, "request: is not JSON" )       ; break ; }  // impossible for HTTP_GET_REQUEST
        if( ! p_root->is_dict() ) { error( htm, "request: root is not DICT" )  ; break ; }  // impossible for HTTP_GET_REQUEST
        gx::json* p_path = ((gx::json::DICT*)p_root)->get_item("Path");
        if( ! p_path )            { error( htm, "request 'Path' field missed") ; break ; }  // impossible for HTTP_GET_REQUEST
        gdom_try_json_path find_path_results(p_path);
        if( ! find_path_results.mp_gdom )
        {
            error( htm, find_path_results.oss.str() );
        }
        else
        {
            if( find_path_results.mp_curr->is_dict() ) {
                htm << "<div class=\"menu\">";
                htm << (( gx::gdom::root() != find_path_results.mp_curr ) ? "ESC-Back "
                                                                          : "  n/a    " );
                htm << "F1-Help F2-Type F3-View F10-Root\n";
                htm << "</div>";
            }
        }
        std::string s_path;
        std::string page_path_str;
        std::string page_prev_str;
        std::list<std::string> page_path;
        std::list<gx::json*>::iterator segm = find_path_results.mp_path->val.begin();
        for(std::size_t i=0; i < find_path_results.current_segment_number; ++i, ++segm)
        {
            gx::json* p_path_item = *segm;
            if (p_path_item->is_cstr()) {
                page_path.push_back(((gx::json::CSTR*)p_path_item)->value);
            }
            else
            {
                page_path.push_back(gx::json_dumps(*segm));
            }
        }
        if( 0 == page_path.size() )
        {
            s_path = "[]";
            page_path_str = "/";
        }
        else
        {
            s_path+="[";
            page_prev_str+="/";
            std::size_t path_item_number = 0;
            for( std::list<std::string>::iterator i = page_path.begin(); i != page_path.end(); ++i )
            {
                path_item_number += 1;
                s_path += std::string("\"") + (*i) + "\",";
                page_path_str += "/" + (*i);
                if ( path_item_number < page_path.size() ) {
                    page_prev_str += (*i) + "/";
                }
            }
            s_path[s_path.size()-1] = std::string("]")[0];
            page_path_str +="/";
        }
        htm << "<div class=\"path\">";
        htm << s_path << "\n";
        htm << "</div>";

        if ( find_path_results.mp_curr->is_dict() )
        {
            gx::dict* p_dict = (gx::dict*)find_path_results.mp_curr;
            if ( gx::gdom::root() != p_dict ) {
                htm << "<a class=\"dict\" href=\"" << page_prev_str << "\">.." << "\n</a>";
            }
            for(gx::dict::iterator i = p_dict->begin(); i != p_dict->end(); ++i )
            {
                htm << "<a class=\"dict\" href=\"" << page_path_str << i->first.c_str() <<  "\">" << i->first.c_str() << "\n</a>";
            }
        }
        //htm << "SESS" << socket()->peerName().toStdString() << ":" << socket()->peerPort() << "\n";
    }
    while(0);

    // Закрываем HTML body, закрываем HTML, cоздаём HttpResponse,
    // Присоединяем к нему HTML и отправляем результат в сокет.
    htm <<"</body></html>";
    std::string msg_body = htm.str();
    QHttpResponseHeader resp(200, "OK");
    resp.setContentLength(msg_body.size());  // response.setValue("Content-Length", message_body.size() ); //response.setValue("Status-Line", "OK");
    resp.setContentType("text/html");
    resp.setValue("Cache-Control","no-cache");   // response.setValue("Date", "Sun, 15 Mar 2009 05:00:48 GMT");
    resp.setValue("Server", "nginx/1.21");       // TODO: is needed ??
    resp.setValue("Connection", "keep-alive");   // Keep socket open
    QString msg = resp.toString() + msg_body.c_str();  // qDebug()<<msg[0]; qDebug()<<msg_body.c_str();  qDebug()<<msg;
    socket()->write(msg.toStdString().c_str());
    return;
}
