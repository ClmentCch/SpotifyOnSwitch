#include "i18n.h"

Lang g_lang = LANG_FR;

#define X(id, fr, en, es, de, it, pt, ja, ko, zh, ru, nl, pl) \
    [id] = { fr, en, es, de, it, pt, ja, ko, zh, ru, nl, pl },

static const char *const TABLE[S_COUNT][LANG_COUNT] = {
    X(S_TAGLINE,
      "Ta musique, sur ta Switch.",
      "Your music, on your Switch.",
      "Tu música, en tu Switch.",
      "Deine Musik, auf deiner Switch.",
      "La tua musica, sul tuo Switch.",
      "A tua música, na tua Switch.",
      "あなたの音楽を、Switch で。",
      "당신의 음악을 Switch에서.",
      "你的音乐，就在 Switch 上。",
      "Твоя музыка — на Switch.",
      "Jouw muziek, op je Switch.",
      "Twoja muzyka na Switchu.")
    X(S_HOME, "Accueil", "Home", "Inicio", "Start", "Home", "Início", "ホーム", "홈", "首页", "Главная", "Home", "Główna")
    X(S_PLAYLISTS, "Playlists", "Playlists", "Listas", "Playlists", "Playlist", "Playlists", "プレイリスト", "재생목록", "播放列表", "Плейлисты", "Playlists", "Playlisty")
    X(S_LIBRARY, "Bibliothèque", "Library", "Biblioteca", "Bibliothek", "Libreria", "Biblioteca", "ライブラリ", "라이브러리", "资料库", "Библиотека", "Bibliotheek", "Biblioteka")
    X(S_SETTINGS, "Paramètres", "Settings", "Ajustes", "Einstellungen", "Impostazioni", "Definições", "設定", "설정", "设置", "Настройки", "Instellingen", "Ustawienia")
    X(S_WELCOME_TITLE,
      "Bienvenue dans Musictt", "Welcome to Musictt", "Bienvenido a Musictt", "Willkommen bei Musictt",
      "Benvenuto in Musictt", "Bem-vindo ao Musictt", "Musictt へようこそ", "Musictt에 오신 것을 환영합니다",
      "欢迎使用 Musictt", "Добро пожаловать в Musictt", "Welkom bij Musictt", "Witamy w Musictt")
    X(S_WELCOME_SUB,
      "Ta bibliothèque musicale, enfin sur Switch.",
      "Your music library, now on Switch.",
      "Tu biblioteca musical, por fin en Switch.",
      "Deine Musikbibliothek, endlich auf der Switch.",
      "La tua libreria musicale, finalmente su Switch.",
      "A tua biblioteca musical, finalmente na Switch.",
      "あなたの音楽ライブラリを Switch で。",
      "당신의 음악 라이브러리를 Switch에서.",
      "你的音乐资料库，终于来到 Switch。",
      "Твоя музыкальная библиотека — на Switch.",
      "Je muziekbibliotheek, eindelijk op de Switch.",
      "Twoja biblioteka muzyczna w końcu na Switchu.")
    X(S_HOME_PLAYLISTS, "Tes playlists", "Your playlists", "Tus listas", "Deine Playlists", "Le tue playlist", "As tuas playlists", "あなたのプレイリスト", "내 재생목록", "你的播放列表", "Твои плейлисты", "Jouw playlists", "Twoje playlisty")
    X(S_HOME_RECENT, "Écoutés récemment", "Recently played", "Reproducido recientemente", "Zuletzt gehört", "Ascoltati di recente", "Ouvidos recentemente", "最近再生した曲", "최근 재생한 곡", "最近播放", "Недавно прослушано", "Onlangs afgespeeld", "Ostatnio odtwarzane")
    X(S_HOME_INFO, "Bibliothèque", "Library", "Biblioteca", "Bibliothek", "Libreria", "Biblioteca", "ライブラリ", "라이브러리", "资料库", "Библиотека", "Bibliotheek", "Biblioteka")
    X(S_RECENT_EMPTY, "Rien pour l'instant.", "Nothing yet.", "Nada por ahora.", "Noch nichts.", "Ancora niente.", "Ainda nada.", "まだ何もありません。", "아직 없습니다.", "暂无内容。", "Пока пусто.", "Nog niets.", "Na razie nic.")
    X(S_TRACKS_COUNT, "%d pistes", "%d tracks", "%d pistas", "%d Titel", "%d brani", "%d faixas", "%d曲", "%d곡", "%d 首", "%d треков", "%d nummers", "%d utworów")
    X(S_PLAYLISTS_COUNT, "%d playlists", "%d playlists", "%d listas", "%d Playlists", "%d playlist", "%d playlists", "%dプレイリスト", "%d개 재생목록", "%d 个播放列表", "%d плейлистов", "%d playlists", "%d playlist")
    X(S_LIBRARY_STATS, "%d playlists · %d pistes", "%d playlists · %d tracks", "%d listas · %d pistas", "%d Playlists · %d Titel", "%d playlist · %d brani", "%d playlists · %d faixas", "%dプレイリスト · %d曲", "%d개 재생목록 · %d곡", "%d 个播放列表 · %d 首", "%d плейлистов · %d треков", "%d playlists · %d nummers", "%d playlist · %d utworów")
    X(S_LIBRARY_EMPTY_TITLE,
      "Ta bibliothèque est vide", "Your library is empty", "Tu biblioteca está vacía", "Deine Bibliothek ist leer",
      "La tua libreria è vuota", "A tua biblioteca está vazia", "ライブラリは空です", "라이브러리가 비어 있습니다",
      "你的资料库是空的", "Твоя библиотека пуста", "Je bibliotheek is leeg", "Twoja biblioteka jest pusta")
    X(S_LIBRARY_EMPTY_BODY,
      "Ajoute tes playlists et tes musiques dans MUSIC pour commencer.",
      "Add your playlists and songs to MUSIC to get started.",
      "Añade tus listas y canciones en MUSIC para empezar.",
      "Füge deine Playlists und Songs in MUSIC hinzu.",
      "Aggiungi playlist e brani in MUSIC per iniziare.",
      "Adiciona playlists e músicas em MUSIC para começar.",
      "MUSIC にプレイリストと曲を追加してください。",
      "MUSIC에 재생목록과 곡을 추가하세요.",
      "将播放列表和歌曲添加到 MUSIC 即可开始。",
      "Добавь плейлисты и треки в MUSIC.",
      "Voeg je playlists en nummers toe aan MUSIC.",
      "Dodaj playlisty i utwory do MUSIC.")
    X(S_PLAYLISTS_EMPTY, "Aucune playlist détectée.", "No playlists found.", "No se encontraron listas.", "Keine Playlists gefunden.", "Nessuna playlist trovata.", "Nenhuma playlist encontrada.", "プレイリストが見つかりません。", "재생목록을 찾을 수 없습니다.", "未找到播放列表。", "Плейлисты не найдены.", "Geen playlists gevonden.", "Nie znaleziono playlist.")
    X(S_PLAYLIST_EMPTY, "Cette playlist est vide.", "This playlist is empty.", "Esta lista está vacía.", "Diese Playlist ist leer.", "Questa playlist è vuota.", "Esta playlist está vazia.", "このプレイリストは空です。", "이 재생목록은 비어 있습니다.", "此播放列表为空。", "Этот плейлист пуст.", "Deze playlist is leeg.", "Ta playlista jest pusta.")
    X(S_BTN_REFRESH, "Actualiser", "Refresh", "Actualizar", "Aktualisieren", "Aggiorna", "Atualizar", "更新", "새로고침", "刷新", "Обновить", "Vernieuwen", "Odśwież")
    X(S_BTN_OK, "OK", "OK", "OK", "OK", "OK", "OK", "OK", "확인", "确定", "ОК", "OK", "OK")
    X(S_BTN_CONFIRM, "Confirmer", "Confirm", "Confirmar", "Bestätigen", "Conferma", "Confirmar", "確認", "확인", "确认", "Подтвердить", "Bevestigen", "Potwierdź")
    X(S_BTN_QUIT, "Quitter Musictt", "Quit Musictt", "Salir de Musictt", "Musictt beenden", "Esci da Musictt", "Sair do Musictt", "Musictt を終了", "Musictt 종료", "退出 Musictt", "Выйти из Musictt", "Musictt afsluiten", "Zamknij Musictt")
    X(S_BTN_RETRY, "Réessayer", "Retry", "Reintentar", "Erneut versuchen", "Riprova", "Tentar novamente", "再試行", "다시 시도", "重试", "Повторить", "Opnieuw", "Ponów")
    X(S_LANG_TITLE, "Choisis ta langue", "Choose your language", "Elige tu idioma", "Wähle deine Sprache", "Scegli la lingua", "Escolhe o teu idioma", "言語を選択", "언어 선택", "选择语言", "Выбери язык", "Kies je taal", "Wybierz język")
    X(S_LANG_BODY, "Tu pourras la changer dans les paramètres.", "You can change it later in settings.", "Puedes cambiarlo en los ajustes.", "Du kannst sie später ändern.", "Puoi cambiarla nelle impostazioni.", "Podes mudá-lo nas definições.", "設定でいつでも変更できます。", "설정에서 언제든지 변경할 수 있습니다.", "之后可以在设置中更改。", "Язык можно изменить в настройках.", "Je kunt dit later wijzigen.", "Możesz to zmienić w ustawieniach.")
    X(S_SD_TITLE,
      "Musictt a préparé ton espace musical !", "Musictt prepared your music space!",
      "¡Musictt preparó tu espacio musical!", "Musictt hat deinen Musikbereich vorbereitet!",
      "Musictt ha preparato il tuo spazio musicale!", "O Musictt preparou o teu espaço musical!",
      "Musictt が音楽スペースを準備しました！", "Musictt가 음악 공간을 준비했습니다!",
      "Musictt 已准备好你的音乐空间！", "Musictt подготовил твоё музыкальное пространство!",
      "Musictt heeft je muziekruimte voorbereid!", "Musictt przygotował twoją przestrzeń muzyczną!")
    X(S_SD_BODY,
      "Ajoute tes musiques et tes playlists sur la carte SD, dans le dossier MUSIC.",
      "Add your songs and playlists to the MUSIC folder on the SD card.",
      "Añade tus canciones y listas a la carpeta MUSIC de la tarjeta SD.",
      "Füge deine Songs und Playlists im Ordner MUSIC auf der SD-Karte hinzu.",
      "Aggiungi brani e playlist nella cartella MUSIC della scheda SD.",
      "Adiciona músicas e playlists à pasta MUSIC do cartão SD.",
      "SD カードの MUSIC フォルダに曲とプレイリストを追加してください。",
      "SD 카드의 MUSIC 폴더에 곡과 재생목록을 추가하세요.",
      "将歌曲和播放列表添加到 SD 卡的 MUSIC 文件夹。",
      "Добавь треки и плейлисты в папку MUSIC на карте SD.",
      "Voeg nummers en playlists toe aan de map MUSIC op de SD-kaart.",
      "Dodaj utwory i playlisty do folderu MUSIC na karcie SD.")
    X(S_SD_HINT,
      "PLAYLISTS/ pour tes playlists, LOGO/ pour les pochettes, CONFIG/ pour les réglages.",
      "PLAYLISTS/ for playlists, LOGO/ for artwork, CONFIG/ for settings.",
      "PLAYLISTS/ para listas, LOGO/ para carátulas, CONFIG/ para ajustes.",
      "PLAYLISTS/ für Playlists, LOGO/ für Cover, CONFIG/ für Einstellungen.",
      "PLAYLISTS/ per le playlist, LOGO/ per le copertine, CONFIG/ per le impostazioni.",
      "PLAYLISTS/ para playlists, LOGO/ para capas, CONFIG/ para definições.",
      "PLAYLISTS/ はプレイリスト、LOGO/ はジャケット、CONFIG/ は設定用です。",
      "PLAYLISTS/는 재생목록, LOGO/는 커버, CONFIG/는 설정용입니다.",
      "PLAYLISTS/ 存放播放列表，LOGO/ 存放封面，CONFIG/ 存放设置。",
      "PLAYLISTS/ — плейлисты, LOGO/ — обложки, CONFIG/ — настройки.",
      "PLAYLISTS/ voor playlists, LOGO/ voor covers, CONFIG/ voor instellingen.",
      "PLAYLISTS/ na playlisty, LOGO/ na okładki, CONFIG/ na ustawienia.")
    X(S_RELAUNCH_TITLE, "Il ne reste qu'une étape", "One last step", "Solo falta un paso", "Nur noch ein Schritt", "Manca solo un passo", "Falta apenas um passo", "あと一歩です", "마지막 단계입니다", "还差一步", "Остался один шаг", "Nog één stap", "Ostatni krok")
    X(S_RELAUNCH_BODY,
      "Quitte Musictt, ajoute tes fichiers dans MUSIC sur la carte SD, puis relance Musictt pour charger ta bibliothèque.",
      "Quit Musictt, add your files to MUSIC on the SD card, then launch Musictt again to load your library.",
      "Sal de Musictt, añade tus archivos a MUSIC en la tarjeta SD y vuelve a iniciar Musictt.",
      "Beende Musictt, füge deine Dateien im Ordner MUSIC auf der SD-Karte hinzu und starte Musictt erneut.",
      "Esci da Musictt, aggiungi i file in MUSIC sulla scheda SD, poi riavvia Musictt.",
      "Sai do Musictt, adiciona os teus ficheiros em MUSIC no cartão SD e volta a abrir o Musictt.",
      "Musictt を終了し、SD カードの MUSIC にファイルを追加してから、Musictt を起動し直してください。",
      "Musictt를 종료하고 SD 카드의 MUSIC에 파일을 추가한 뒤 Musictt를 다시 실행하세요.",
      "退出 Musictt，将文件添加到 SD 卡的 MUSIC 中，然后重新打开 Musictt 以加载资料库。",
      "Выйди из Musictt, добавь файлы в MUSIC на карте SD и запусти Musictt снова.",
      "Sluit Musictt af, voeg je bestanden toe aan MUSIC op de SD-kaart en start Musictt opnieuw.",
      "Zamknij Musictt, dodaj pliki do MUSIC na karcie SD i uruchom Musictt ponownie.")
    X(S_NOW_PLAYING, "Lecture en cours", "Now playing", "Reproduciendo", "Wird abgespielt", "In riproduzione", "A reproduzir", "再生中", "재생 중", "正在播放", "Сейчас играет", "Nu aan het spelen", "Teraz odtwarzane")
    X(S_VOLUME, "Volume", "Volume", "Volumen", "Lautstärke", "Volume", "Volume", "音量", "볼륨", "音量", "Громкость", "Volume", "Głośność")
    X(S_SET_LANGUAGE, "Langue", "Language", "Idioma", "Sprache", "Lingua", "Idioma", "言語", "언어", "语言", "Язык", "Taal", "Język")
    X(S_SET_MOTION, "Animations", "Animations", "Animaciones", "Animationen", "Animazioni", "Animações", "アニメーション", "애니메이션", "动画", "Анимации", "Animaties", "Animacje")
    X(S_SET_REFRESH, "Actualiser la bibliothèque", "Refresh library", "Actualizar biblioteca", "Bibliothek aktualisieren", "Aggiorna libreria", "Atualizar biblioteca", "ライブラリを更新", "라이브러리 새로고침", "刷新资料库", "Обновить библиотеку", "Bibliotheek vernieuwen", "Odśwież bibliotekę")
    X(S_SET_ABOUT, "À propos", "About", "Acerca de", "Über", "Informazioni", "Sobre", "このアプリについて", "정보", "关于", "О приложении", "Over", "O aplikacji")
    X(S_REFRESHED, "Bibliothèque actualisée", "Library refreshed", "Biblioteca actualizada", "Bibliothek aktualisiert", "Libreria aggiornata", "Biblioteca atualizada", "ライブラリを更新しました", "라이브러리를 새로고침했습니다", "资料库已刷新", "Библиотека обновлена", "Bibliotheek vernieuwd", "Biblioteka odświeżona")
    X(S_FORMATS, "MP3 · WAV · FLAC · OGG", "MP3 · WAV · FLAC · OGG", "MP3 · WAV · FLAC · OGG", "MP3 · WAV · FLAC · OGG", "MP3 · WAV · FLAC · OGG", "MP3 · WAV · FLAC · OGG", "MP3 · WAV · FLAC · OGG", "MP3 · WAV · FLAC · OGG", "MP3 · WAV · FLAC · OGG", "MP3 · WAV · FLAC · OGG", "MP3 · WAV · FLAC · OGG", "MP3 · WAV · FLAC · OGG")
    X(S_STORAGE, "sdmc:/MUSIC", "sdmc:/MUSIC", "sdmc:/MUSIC", "sdmc:/MUSIC", "sdmc:/MUSIC", "sdmc:/MUSIC", "sdmc:/MUSIC", "sdmc:/MUSIC", "sdmc:/MUSIC", "sdmc:/MUSIC", "sdmc:/MUSIC", "sdmc:/MUSIC")
    X(S_ON, "Activées", "On", "Activadas", "An", "Attive", "Ativadas", "オン", "켜짐", "开启", "Вкл.", "Aan", "Włączone")
    X(S_OFF, "Désactivées", "Off", "Desactivadas", "Aus", "Disattivate", "Desativadas", "オフ", "꺼짐", "关闭", "Выкл.", "Uit", "Wyłączone")
    X(S_LOADING, "Chargement…", "Loading…", "Cargando…", "Lädt…", "Caricamento…", "A carregar…", "読み込み中…", "불러오는 중…", "加载中…", "Загрузка…", "Laden…", "Ładowanie…")
    X(S_PLAYING, "Lecture", "Playing", "Reproduciendo", "Wiedergabe", "In riproduzione", "A reproduzir", "再生中", "재생 중", "播放中", "Играет", "Speelt", "Odtwarzanie")
    X(S_PAUSED, "En pause", "Paused", "En pausa", "Pausiert", "In pausa", "Em pausa", "一時停止", "일시정지", "已暂停", "Пауза", "Gepauzeerd", "Wstrzymane")
    X(S_ERR_SD, "Carte SD inaccessible", "SD card unavailable", "Tarjeta SD no disponible", "SD-Karte nicht verfügbar", "Scheda SD non disponibile", "Cartão SD indisponível", "SD カードにアクセスできません", "SD 카드에 접근할 수 없습니다", "无法访问 SD 卡", "Карта SD недоступна", "SD-kaart niet beschikbaar", "Karta SD niedostępna")
    X(S_ERR_SD_BODY,
      "Vérifie que la carte SD est insérée, puis réessaie.",
      "Check that the SD card is inserted, then try again.",
      "Comprueba que la tarjeta SD está insertada y vuelve a intentarlo.",
      "Prüfe, ob die SD-Karte eingelegt ist, und versuche es erneut.",
      "Controlla che la scheda SD sia inserita, poi riprova.",
      "Verifica se o cartão SD está inserido e tenta novamente.",
      "SD カードが挿入されているか確認して、もう一度お試しください。",
      "SD 카드가 삽입되어 있는지 확인한 후 다시 시도하세요.",
      "请确认 SD 卡已插入，然后重试。",
      "Проверь, вставлена ли карта SD, и повтори попытку.",
      "Controleer of de SD-kaart is geplaatst en probeer opnieuw.",
      "Sprawdź, czy karta SD jest włożona, i spróbuj ponownie.")
    X(S_ERR_JSON, "package.json invalide", "Invalid package.json", "package.json no válido", "Ungültige package.json", "package.json non valido", "package.json inválido", "package.json が無効です", "package.json이 잘못되었습니다", "package.json 无效", "Неверный package.json", "Ongeldige package.json", "Nieprawidłowy package.json")
    X(S_ERR_JSON_BODY,
      "Vérifie la syntaxe du package.json de cette playlist.",
      "Check the syntax of this playlist's package.json.",
      "Revisa la sintaxis del package.json de esta lista.",
      "Prüfe die Syntax der package.json dieser Playlist.",
      "Controlla la sintassi del package.json di questa playlist.",
      "Verifica a sintaxe do package.json desta playlist.",
      "このプレイリストの package.json の書式を確認してください。",
      "이 재생목록의 package.json 형식을 확인하세요.",
      "请检查此播放列表的 package.json 格式。",
      "Проверь синтаксис package.json этого плейлиста.",
      "Controleer de syntax van de package.json van deze playlist.",
      "Sprawdź składnię package.json tej playlisty.")
    X(S_ERR_AUDIO, "Audio indisponible", "Audio unavailable", "Audio no disponible", "Audio nicht verfügbar", "Audio non disponibile", "Áudio indisponível", "オーディオを利用できません", "오디오를 사용할 수 없습니다", "音频不可用", "Аудио недоступно", "Audio niet beschikbaar", "Audio niedostępne")
    X(S_ERR_AUDIO_BODY,
      "La sortie audio n'a pas pu s'ouvrir : la lecture est désactivée.",
      "The audio output could not be opened: playback is disabled.",
      "No se pudo abrir la salida de audio: la reproducción está desactivada.",
      "Die Audioausgabe konnte nicht geöffnet werden: Wiedergabe deaktiviert.",
      "Impossibile aprire l'uscita audio: riproduzione disattivata.",
      "Não foi possível abrir a saída de áudio: reprodução desativada.",
      "オーディオ出力を開けませんでした。再生は無効です。",
      "오디오 출력을 열 수 없습니다. 재생이 비활성화됩니다.",
      "无法打开音频输出：播放已禁用。",
      "Не удалось открыть аудиовыход: воспроизведение отключено.",
      "De audio-uitgang kon niet worden geopend: afspelen is uitgeschakeld.",
      "Nie można otworzyć wyjścia audio: odtwarzanie jest wyłączone.")
    X(S_ERR_TRACK_MISSING, "Fichier audio introuvable : %s", "Audio file not found: %s", "Archivo de audio no encontrado: %s", "Audiodatei nicht gefunden: %s", "File audio non trovato: %s", "Ficheiro de áudio não encontrado: %s", "音声ファイルが見つかりません: %s", "오디오 파일을 찾을 수 없습니다: %s", "找不到音频文件：%s", "Аудиофайл не найден: %s", "Audiobestand niet gevonden: %s", "Nie znaleziono pliku audio: %s")
    X(S_ERR_TRACK_FORMAT, "Format non pris en charge : %s", "Unsupported format: %s", "Formato no compatible: %s", "Nicht unterstütztes Format: %s", "Formato non supportato: %s", "Formato não suportado: %s", "未対応の形式です: %s", "지원하지 않는 형식입니다: %s", "不支持的格式：%s", "Формат не поддерживается: %s", "Niet-ondersteund formaat: %s", "Nieobsługiwany format: %s")
    X(S_ERR_TRACK_READ, "Impossible de lire ce fichier : %s", "Cannot play this file: %s", "No se puede reproducir este archivo: %s", "Diese Datei kann nicht abgespielt werden: %s", "Impossibile riprodurre questo file: %s", "Não é possível reproduzir este ficheiro: %s", "このファイルを再生できません: %s", "이 파일을 재생할 수 없습니다: %s", "无法播放此文件：%s", "Не удалось воспроизвести файл: %s", "Kan dit bestand niet afspelen: %s", "Nie można odtworzyć tego pliku: %s")
    X(S_ERR_TRACK_LONG, "Fichier trop long pour la mémoire disponible : %s", "File too long for the available memory: %s", "Archivo demasiado largo para la memoria disponible: %s", "Datei zu lang für den verfügbaren Speicher: %s", "File troppo lungo per la memoria disponibile: %s", "Ficheiro demasiado longo para a memória disponível: %s", "ファイルが長すぎてメモリに収まりません: %s", "파일이 너무 길어 메모리에 담을 수 없습니다: %s", "文件太长，内存不足：%s", "Файл слишком длинный для доступной памяти: %s", "Bestand te lang voor het beschikbare geheugen: %s", "Plik zbyt długi dla dostępnej pamięci: %s")
    X(S_TOAST_SCANNING, "Analyse de la carte SD…", "Scanning the SD card…", "Analizando la tarjeta SD…", "SD-Karte wird gelesen…", "Analisi della scheda SD…", "A analisar o cartão SD…", "SD カードを読み込んでいます…", "SD 카드를 검사하는 중…", "正在扫描 SD 卡…", "Сканирование карты SD…", "SD-kaart wordt gelezen…", "Skanowanie karty SD…")
    X(S_TOAST_PLAYLIST_ERROR, "Playlist ignorée : %s", "Playlist skipped: %s", "Lista omitida: %s", "Playlist übersprungen: %s", "Playlist ignorata: %s", "Playlist ignorada: %s", "プレイリストをスキップしました: %s", "재생목록을 건너뛰었습니다: %s", "已跳过播放列表：%s", "Плейлист пропущен: %s", "Playlist overgeslagen: %s", "Pominięto playlistę: %s")
    X(S_ABOUT_BODY,
      "Lecteur musical natif pour Nintendo Switch. Décode le MP3, le WAV, le FLAC et l'OGG directement sur la console.",
      "A native music player for Nintendo Switch. Decodes MP3, WAV, FLAC and OGG on the console.",
      "Un reproductor musical nativo para Nintendo Switch. Decodifica MP3, WAV, FLAC y OGG en la consola.",
      "Ein nativer Musikplayer für die Nintendo Switch. Dekodiert MP3, WAV, FLAC und OGG direkt auf der Konsole.",
      "Un lettore musicale nativo per Nintendo Switch. Decodifica MP3, WAV, FLAC e OGG sulla console.",
      "Um leitor de música nativo para a Nintendo Switch. Descodifica MP3, WAV, FLAC e OGG na consola.",
      "Nintendo Switch 用のネイティブ音楽プレーヤー。MP3・WAV・FLAC・OGG を本体で再生します。",
      "Nintendo Switch용 네이티브 음악 플레이어입니다. MP3, WAV, FLAC, OGG를 콘솔에서 재생합니다.",
      "适用于 Nintendo Switch 的原生音乐播放器，可在主机上解码 MP3、WAV、FLAC 和 OGG。",
      "Нативный музыкальный плеер для Nintendo Switch. Декодирует MP3, WAV, FLAC и OGG прямо на консоли.",
      "Een native muziekspeler voor de Nintendo Switch. Decodeert MP3, WAV, FLAC en OGG op de console.",
      "Natywny odtwarzacz muzyki na Nintendo Switch. Dekoduje MP3, WAV, FLAC i OGG na konsoli.")
    X(S_SONGS, "Titres", "Songs", "Canciones", "Titel", "Brani", "Músicas", "曲", "곡", "歌曲", "Треки", "Nummers", "Utwory")
    X(S_PLAY_ALL, "Tout lire", "Play all", "Reproducir todo", "Alle abspielen", "Riproduci tutto", "Reproduzir tudo", "すべて再生", "모두 재생", "全部播放", "Играть всё", "Alles afspelen", "Odtwórz wszystko")
    X(S_MISSING, "Fichier manquant", "File missing", "Archivo ausente", "Datei fehlt", "File mancante", "Ficheiro em falta", "ファイルがありません", "파일 없음", "文件缺失", "Файл отсутствует", "Bestand ontbreekt", "Brak pliku")
};

#undef X

const char *i18n_get(StrId id) {
    return i18n_get_lang(id, g_lang);
}

const char *i18n_get_lang(StrId id, Lang lang) {
    if (id < 0 || id >= S_COUNT) return "";
    if (lang < 0 || lang >= LANG_COUNT) lang = LANG_EN;
    const char *s = TABLE[id][lang];
    return s ? s : "";
}

static char num_buf[8][192];
static int num_slot;

const char *i18n_num(StrId id, int value) {
    const char *fmt = i18n_get(id);
    char *buf = num_buf[num_slot];
    num_slot = (num_slot + 1) % 8;
    snprintf(buf, sizeof(num_buf[0]), fmt, value);
    return buf;
}

const char *i18n_num2(StrId id, int first, int second) {
    const char *fmt = i18n_get(id);
    char *buf = num_buf[num_slot];
    num_slot = (num_slot + 1) % 8;
    snprintf(buf, sizeof(num_buf[0]), fmt, first, second);
    return buf;
}

const char *i18n_str(StrId id, const char *value) {
    const char *fmt = i18n_get(id);
    char *buf = num_buf[num_slot];
    num_slot = (num_slot + 1) % 8;
    snprintf(buf, sizeof(num_buf[0]), fmt, value ? value : "");
    return buf;
}

static const char *const NATIVE[LANG_COUNT] = {
    "Français", "English", "Español", "Deutsch", "Italiano", "Português",
    "日本語", "한국어", "中文", "Русский", "Nederlands", "Polski"
};
static const char *const CODES[LANG_COUNT] = {
    "fr", "en", "es", "de", "it", "pt", "ja", "ko", "zh", "ru", "nl", "pl"
};

const char *i18n_native_name(Lang lang) {
    if (lang < 0 || lang >= LANG_COUNT) return "";
    return NATIVE[lang];
}

const char *i18n_code(Lang lang) {
    if (lang < 0 || lang >= LANG_COUNT) return "";
    return CODES[lang];
}

int i18n_uses_cjk(Lang lang) {
    return lang == LANG_JA || lang == LANG_KO || lang == LANG_ZH;
}

int i18n_cjk_face(Lang lang) {
    switch (lang) {
        case LANG_KO: return 1;  /* Noto Sans CJK KR */
        case LANG_ZH: return 2;  /* Noto Sans CJK SC */
        default:      return 0;  /* Noto Sans CJK JP, also covers Latin */
    }
}
