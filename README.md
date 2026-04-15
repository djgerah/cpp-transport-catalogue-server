# Transport Catalogue Server

HTTP-сервер для работы с транспортным справочником.
Поддерживает хранение информации об остановках и маршрутах, обработку запросов, визуализацию карты и обновление данных.

Проект состоит из двух частей:

* **Transport Catalogue** — ядро, которое хранит информацию об остановках, автобусах, расстояниях, строит статистику маршрутов.
* **HTTP Server** — обертка над транспортным справочником, позволяющая работать с ним через REST API.

---

## Возможности

* **Transport Catalogue**

  * хранение остановок с координатами;
  * хранение автобусных маршрутов;
  * вычисление статистики маршрутов (количество остановок, длина, кривизна);
  * учет расстояний между остановками.
* **HTTP API**

  * `POST /load` — загрузка данных (`base_requests`, `render_settings`, `routing_settings`);
  * `POST /query` — выполнение `stat_requests` (Bus, Stop, Map, Route);
  * `GET /map` — рендер карты маршрутов в формате SVG;
  * `PUT /stop` — добавление остановки;
  * `DELETE /stop` — удаление остановки;
  * `PATCH /bus` — частичное обновление (например, добавление остановок в маршрут).
  * `PATCH /catalogue` — частичное обновление (например, удаление остановок из каталога).

---

## Примеры запросов

### Загрузка каталога

```http
POST /load
Content-Type: application/json

{
  "base_requests": [
    {
      "type": "Bus",
      "name": "37",
      "stops": [
        "Остановка №1",
        "Остановка №2",
        "Остановка №3",
        "Остановка №4",
        "Остановка №5",
        "Остановка №6",
        "Остановка №7",
        "Остановка №8",
        "Остановка №9",
        "Остановка №10",
        "Остановка №11",
        "Остановка №12",
        "Остановка №13",
        "Остановка №14"
      ],
      "is_roundtrip": false
    },
    {
      "type": "Stop",
      "name": "Остановка №1",
      "latitude": 55.690431,
      "longitude": 38.113266,
      "road_distances": { "Остановка №2": 1500 }
    },
    {
      "type": "Stop",
      "name": "Остановка №2",
      "latitude": 55.682256,
      "longitude": 38.116579,
      "road_distances": { "Остановка №1": 1500, "Остановка №3": 1200 }
    },
    {
      "type": "Stop",
      "name": "Остановка №3",
      "latitude": 55.679148,
      "longitude": 38.110804,
      "road_distances": { "Остановка №2": 1200, "Остановка №4": 800 }
    },
    {
      "type": "Stop",
      "name": "Остановка №4",
      "latitude": 55.656141,
      "longitude": 38.100587,
      "road_distances": { "Остановка №3": 800, "Остановка №5": 1000 }
    },
    {
      "type": "Stop",
      "name": "Остановка №5",
      "latitude": 55.654301,
      "longitude": 38.093509,
      "road_distances": { "Остановка №4": 1000, "Остановка №6": 1500 }
    },
    {
      "type": "Stop",
      "name": "Остановка №6",
      "latitude": 55.656124,
      "longitude": 38.071597,
      "road_distances": { "Остановка №5": 1500, "Остановка №7": 1200 }
    },
    {
      "type": "Stop",
      "name": "Остановка №7",
      "latitude": 55.658625,
      "longitude": 38.035084,
      "road_distances": { "Остановка №6": 1200, "Остановка №8": 1000 }
    },
    {
      "type": "Stop",
      "name": "Остановка №8",
      "latitude": 55.661347,
      "longitude": 38.011089,
      "road_distances": { "Остановка №7": 1000, "Остановка №9": 400 }
    },
    {
      "type": "Stop",
      "name": "Остановка №9",
      "latitude": 55.657520,
      "longitude": 38.011055,
      "road_distances": { "Остановка №8": 400, "Остановка №10": 300 }
    },
    {
      "type": "Stop",
      "name": "Остановка №10",
      "latitude": 55.654491,
      "longitude": 38.011085,
      "road_distances": { "Остановка №9": 300, "Остановка №11": 400 }
    },
    {
      "type": "Stop",
      "name": "Остановка №11",
      "latitude": 55.646084,
      "longitude": 38.005671,
      "road_distances": { "Остановка №10": 400, "Остановка №12": 700 }
    },
    {
      "type": "Stop",
      "name": "Остановка №12",
      "latitude": 55.641199,
      "longitude": 38.024770,
      "road_distances": { "Остановка №11": 700, "Остановка №13": 500 }
    },
    {
      "type": "Stop",
      "name": "Остановка №13",
      "latitude": 55.644166,
      "longitude": 38.032845,
      "road_distances": { "Остановка №12": 500, "Остановка №14": 600 }
    },
    {
      "type": "Stop",
      "name": "Остановка №14",
      "latitude": 55.646193,
      "longitude": 38.039987,
      "road_distances": { "Остановка №13": 600 }
    }
  ],
  "render_settings": {
    "width": 1000,
    "height": 1000,
    "padding": 30,
    "stop_radius": 5,
    "line_width": 14,
    "bus_label_font_size": 14,
    "bus_label_offset": [10, -10],
    "stop_label_font_size": 14,
    "stop_label_offset": [15, 10],
    "underlayer_color": [255,255,255,0.85],
    "underlayer_width": 3,
    "color_palette": ["green", [255,160,0],"red"]
  },
  "routing_settings": {
    "bus_velocity": 21,
    "bus_wait_time": 2
  }
}
```

### Добавление остановки

```http
PUT /stop
Content-Type: application/json

{
  "base_requests": [
    {
      "type": "Stop",
      "name": "Остановка №15",
      "latitude": 55.667273,
      "longitude": 38.103734,
      "road_distances": {
        "Остановка №3": 2500,
        "Остановка №4": 500
      }
    }
  ]
}
```

### Добавление маршрута

```http
PUT /bus
Content-Type: application/json

{
  "base_requests": [
    {
      "type": "Bus",
      "name": "38",
      "stops": [
        "Остановка №1",
        "Остановка №2",
        "Остановка №3"
      ],
      "is_roundtrip": false
    }
  ]
}
```

### Частичное обновление маршрута

```http
PATCH /patch
Content-Type: application/json

{
  "base_requests": [
    {
      "type": "Bus",
      "name": "37",
      "stops": [
        "Остановка №15"
      ],
      "position": 3,
      "is_roundtrip": false
    }
  ]
}
```

### Удаление остановки

```http
DELETE /stop
Content-Type: application/json

{
  "delete_requests": [
    {
      "type": "Bus",
      "name": "37",
      "operation": "delete",
      "stops": ["Остановка №15"]
    }
  ]
}
```

### Выполнение запроса (статистика по маршруту)

```http
POST /query
Content-Type: application/json

{
  "stat_requests": [
    {
      "id": 1,
      "name": "37",
      "type": "Bus"
    }
  ]
}
```

### Получение карты

```http
GET /map
```

Ответ: SVG-документ.
Пример визуализации карты маршрутов:

![Пример карты маршрутов](data/map_example.png)

---

## Сборка и запуск

```bash
make build && make run
```

Сервер запустится на [http://localhost:8080](http://localhost:8080).

---

## Зависимости

* C++17
* [C++ REST SDK](https://github.com/microsoft/cpprestsdk) — HTTP сервер
* Встроенный JSON-парсер
* SVG-рендер для визуализации карты

---

## Примечание

* Все запросы выполняются в формате application/json.
* Для GET /map возвращается image/svg+xml.
* После POST /load можно безопасно выполнять PUT, PATCH и POST /query.
* Взаимодействие протестировано через Postman.

---

🚀 `transport-catalogue` можно использовать как самостоятельную библиотеку для работы с транспортными данными или как сервер через REST API.
