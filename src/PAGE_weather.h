const char PAGE_Weather[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Wetterstation</title>
  <style>
    :root {
      --bg-top: #f3f8fb;
      --bg-bottom: #dfeaf2;
      --card: rgba(255, 255, 255, 0.82);
      --card-strong: rgba(255, 255, 255, 0.94);
      --border: rgba(35, 67, 95, 0.12);
      --text: #173042;
      --muted: #6a7d8d;
      --accent: #0f7f9b;
      --accent-strong: #0f5f8a;
      --warm: #e17e39;
      --good: #258b5d;
      --warn: #cb4b42;
      --shadow: 0 24px 60px rgba(16, 43, 66, 0.12);
      --radius: 24px;
    }

    * {
      box-sizing: border-box;
    }

    html, body {
      margin: 0;
      min-height: 100%;
    }

    body {
      font-family: "Avenir Next", "Segoe UI", "Helvetica Neue", Arial, sans-serif;
      color: var(--text);
      background:
        radial-gradient(circle at top left, rgba(15, 127, 155, 0.18), transparent 32%),
        radial-gradient(circle at top right, rgba(225, 126, 57, 0.16), transparent 28%),
        linear-gradient(180deg, var(--bg-top) 0%, var(--bg-bottom) 100%);
    }

    .page {
      width: min(980px, calc(100% - 28px));
      margin: 0 auto;
      padding: 22px 0 34px;
    }

    .hero,
    .card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      box-shadow: var(--shadow);
      backdrop-filter: blur(18px);
    }

    .hero {
      position: relative;
      overflow: hidden;
      padding: 24px 24px 20px;
      margin-bottom: 18px;
    }

    .hero::after {
      content: "";
      position: absolute;
      inset: auto -40px -70px auto;
      width: 220px;
      height: 220px;
      border-radius: 50%;
      background: radial-gradient(circle, rgba(15, 127, 155, 0.18), transparent 65%);
      pointer-events: none;
    }

    .hero-top {
      display: flex;
      justify-content: space-between;
      gap: 18px;
      align-items: flex-start;
      margin-bottom: 24px;
    }

    .eyebrow {
      margin: 0 0 8px;
      color: var(--accent-strong);
      font-size: 0.78rem;
      font-weight: 700;
      letter-spacing: 0.18em;
      text-transform: uppercase;
    }

    h1 {
      margin: 0;
      font-size: clamp(2rem, 5vw, 3.3rem);
      line-height: 0.95;
      letter-spacing: -0.04em;
    }

    .hero-copy {
      margin: 12px 0 0;
      max-width: 34rem;
      color: var(--muted);
      font-size: 0.98rem;
      line-height: 1.5;
    }

    .live-pill {
      display: inline-flex;
      align-items: center;
      gap: 10px;
      padding: 10px 14px;
      border-radius: 999px;
      border: 1px solid rgba(15, 95, 138, 0.15);
      background: rgba(255, 255, 255, 0.7);
      color: var(--accent-strong);
      font-size: 0.9rem;
      font-weight: 700;
      white-space: nowrap;
    }

    .live-dot {
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background: var(--good);
      box-shadow: 0 0 0 6px rgba(37, 139, 93, 0.14);
      animation: pulse 2.2s infinite;
    }

    .hero-metrics {
      display: grid;
      grid-template-columns: repeat(3, minmax(0, 1fr));
      gap: 12px;
    }

    .hero-stat {
      padding: 14px 16px;
      border-radius: 18px;
      background: var(--card-strong);
      border: 1px solid rgba(35, 67, 95, 0.08);
    }

    .hero-stat-label {
      display: block;
      margin-bottom: 7px;
      color: var(--muted);
      font-size: 0.8rem;
      text-transform: uppercase;
      letter-spacing: 0.08em;
    }

    .hero-stat-value {
      font-size: clamp(1.2rem, 3vw, 1.8rem);
      font-weight: 800;
      letter-spacing: -0.03em;
      font-variant-numeric: tabular-nums;
    }

    .status-good {
      color: var(--good);
    }

    .status-warn {
      color: var(--warn);
    }

    .status-neutral {
      color: var(--accent-strong);
    }

    .alert {
      display: none;
      margin-bottom: 18px;
      padding: 13px 16px;
      border-radius: 18px;
      border: 1px solid rgba(203, 75, 66, 0.18);
      background: rgba(255, 240, 238, 0.88);
      color: var(--warn);
      box-shadow: 0 12px 28px rgba(203, 75, 66, 0.08);
    }

    .grid {
      display: grid;
      grid-template-columns: repeat(12, minmax(0, 1fr));
      gap: 18px;
    }

    .sensor-card {
      grid-column: span 6;
      padding: 20px;
    }

    .insight-card {
      grid-column: span 12;
      padding: 22px;
      background: linear-gradient(135deg, rgba(15, 127, 155, 0.94), rgba(18, 53, 85, 0.96));
      color: #f4fbff;
    }

    .sensor-head,
    .insight-head {
      display: flex;
      justify-content: space-between;
      align-items: center;
      gap: 12px;
    }

    .sensor-title,
    .insight-title {
      margin: 0;
      font-size: 1rem;
      letter-spacing: 0.04em;
      text-transform: uppercase;
    }

    .sensor-badge {
      padding: 7px 10px;
      border-radius: 999px;
      background: rgba(15, 127, 155, 0.1);
      color: var(--accent-strong);
      font-size: 0.78rem;
      font-weight: 700;
    }

    .sensor-temp {
      margin: 18px 0 4px;
      font-size: clamp(2.4rem, 6vw, 4rem);
      font-weight: 800;
      letter-spacing: -0.05em;
      line-height: 0.92;
      font-variant-numeric: tabular-nums;
    }

    .sensor-temp-sub {
      margin: 0 0 18px;
      color: var(--muted);
      font-size: 0.92rem;
    }

    .rows {
      display: grid;
      gap: 12px;
    }

    .row {
      display: flex;
      justify-content: space-between;
      gap: 14px;
      align-items: center;
      padding: 12px 0;
      border-top: 1px solid rgba(35, 67, 95, 0.08);
    }

    .row:first-child {
      border-top: 0;
      padding-top: 0;
    }

    .row-label {
      color: var(--muted);
      font-size: 0.94rem;
    }

    .row-value {
      font-weight: 700;
      font-variant-numeric: tabular-nums;
      text-align: right;
    }

    .insight-copy {
      margin: 10px 0 20px;
      max-width: 46rem;
      color: rgba(244, 251, 255, 0.78);
      line-height: 1.5;
    }

    .insight-grid {
      display: grid;
      grid-template-columns: minmax(0, 1.3fr) minmax(220px, 0.7fr);
      gap: 18px;
      align-items: stretch;
    }

    .recommendation {
      padding: 18px;
      border-radius: 22px;
      background: rgba(255, 255, 255, 0.12);
      border: 1px solid rgba(255, 255, 255, 0.12);
    }

    .recommendation-label {
      display: block;
      margin-bottom: 8px;
      color: rgba(244, 251, 255, 0.74);
      font-size: 0.78rem;
      letter-spacing: 0.12em;
      text-transform: uppercase;
    }

    .recommendation-value {
      font-size: clamp(1.4rem, 3vw, 2.1rem);
      font-weight: 800;
      line-height: 1.05;
      letter-spacing: -0.04em;
    }

    .recommendation-note {
      margin-top: 10px;
      color: rgba(244, 251, 255, 0.74);
      font-size: 0.95rem;
      line-height: 1.45;
    }

    .difference-box {
      display: grid;
      align-content: space-between;
      gap: 18px;
      padding: 18px;
      border-radius: 22px;
      background: rgba(7, 22, 36, 0.16);
      border: 1px solid rgba(255, 255, 255, 0.1);
    }

    .difference-value {
      font-size: clamp(2rem, 4vw, 3rem);
      font-weight: 800;
      letter-spacing: -0.05em;
      font-variant-numeric: tabular-nums;
    }

    .difference-scale {
      position: relative;
      height: 10px;
      border-radius: 999px;
      background: rgba(255, 255, 255, 0.16);
      overflow: hidden;
    }

    .difference-fill {
      height: 100%;
      width: 0%;
      border-radius: inherit;
      background: linear-gradient(90deg, #73d8ad 0%, #cfe989 35%, #ffcc6d 68%, #ff826d 100%);
      transition: width 280ms ease;
    }

    .difference-meta {
      display: flex;
      justify-content: space-between;
      gap: 12px;
      color: rgba(244, 251, 255, 0.78);
      font-size: 0.88rem;
    }

    .fade {
      animation: fade-up 0.5s ease both;
    }

    @keyframes pulse {
      0%, 100% { transform: scale(1); opacity: 1; }
      50% { transform: scale(1.08); opacity: 0.8; }
    }

    @keyframes fade-up {
      from {
        opacity: 0;
        transform: translateY(10px);
      }
      to {
        opacity: 1;
        transform: translateY(0);
      }
    }

    @media (max-width: 760px) {
      .hero-top,
      .sensor-head,
      .insight-head,
      .row,
      .difference-meta {
        flex-direction: column;
        align-items: flex-start;
      }

      .hero-metrics,
      .insight-grid {
        grid-template-columns: 1fr;
      }

      .sensor-card,
      .insight-card {
        grid-column: span 12;
      }

      .row-value {
        text-align: left;
      }
    }
  </style>
</head>
<body>
  <main class="page">
    <section class="hero fade">
      <div class="hero-top">
        <div>
          <p class="eyebrow">Lokal · Live · Klima</p>
          <h1>Wetterstation</h1>
          <p class="hero-copy">
            Live-Ansicht der Innen- und Außendaten. Die Karte unten bewertet, ob trockenere Außenluft gerade für sinnvolleres Lüften spricht.
          </p>
        </div>
        <div class="live-pill">
          <span class="live-dot"></span>
          <span id="connectionState">Aktualisiert</span>
        </div>
      </div>

      <div class="hero-metrics">
        <div class="hero-stat">
          <span class="hero-stat-label">Lüftung</span>
          <span id="ventingHeadline" class="hero-stat-value status-neutral">Prüfe Daten</span>
        </div>
        <div class="hero-stat">
          <span class="hero-stat-label">Differenz</span>
          <span id="heroDifference" class="hero-stat-value">--</span>
        </div>
        <div class="hero-stat">
          <span class="hero-stat-label">Stand</span>
          <span id="lastRefreshTime" class="hero-stat-value">--:--</span>
        </div>
      </div>
    </section>

    <div id="error" class="alert fade"></div>

    <section class="grid">
      <article class="card sensor-card fade">
        <div class="sensor-head">
          <h2 class="sensor-title">Innen</h2>
          <span class="sensor-badge">Raumklima</span>
        </div>
        <div id="indoorTemperatureCelsius" class="sensor-temp">--</div>
        <p class="sensor-temp-sub">Temperatur im Innenraum</p>
        <div class="rows">
          <div class="row">
            <span class="row-label">Relative Feuchtigkeit</span>
            <span id="indoorHumidityPercent" class="row-value">--</span>
          </div>
          <div class="row">
            <span class="row-label">Absolute Feuchtigkeit</span>
            <span id="indoorAbsoluteHumidityGm3" class="row-value">--</span>
          </div>
          <div class="row">
            <span class="row-label">Taupunkt</span>
            <span id="indoorDewPointCelsius" class="row-value">--</span>
          </div>
        </div>
      </article>

      <article class="card sensor-card fade">
        <div class="sensor-head">
          <h2 class="sensor-title">Außen</h2>
          <span class="sensor-badge">Funksensor</span>
        </div>
        <div id="outdoorTemperatureCelsius" class="sensor-temp">--</div>
        <p class="sensor-temp-sub">Temperatur ausserhalb des Hauses</p>
        <div class="rows">
          <div class="row">
            <span class="row-label">Relative Feuchtigkeit</span>
            <span id="outdoorHumidityPercent" class="row-value">--</span>
          </div>
          <div class="row">
            <span class="row-label">Absolute Feuchtigkeit</span>
            <span id="outdoorAbsoluteHumidityGm3" class="row-value">--</span>
          </div>
          <div class="row">
            <span class="row-label">Batterie</span>
            <span id="outdoorBatteryOk" class="row-value">--</span>
          </div>
          <div class="row">
            <span class="row-label">Letzte Aktualisierung</span>
            <span id="outdoorSecondsSinceLastReading" class="row-value">--</span>
          </div>
        </div>
      </article>

      <article class="card insight-card fade">
        <div class="insight-head">
          <h2 class="insight-title">Lüftungsempfehlung</h2>
        </div>
        <p class="insight-copy">
          Die Entscheidung basiert auf der absoluten Feuchtigkeit innen gegen außen. Je deutlicher
          die Außenluft trockener ist, desto sinnvoller ist Lüften.
        </p>

        <div class="insight-grid">
          <div class="recommendation">
            <span class="recommendation-label">Aktuelle Einschätzung</span>
            <div id="ventingRecommendation" class="recommendation-value">Warte auf Daten</div>
            <div id="ventingNote" class="recommendation-note">
              Die Seite aktualisiert sich automatisch alle 5 Sekunden.
            </div>
          </div>

          <div class="difference-box">
            <div>
              <span class="recommendation-label">Absolute Differenz</span>
              <div id="absoluteHumidityDifferenceGm3" class="difference-value">--</div>
            </div>
            <div class="difference-scale">
              <div id="differenceFill" class="difference-fill"></div>
            </div>
            <div class="difference-meta">
              <span id="differenceLabel">Neutral</span>
              <span>Schwelle: 3.0 g/m³</span>
            </div>
          </div>
        </div>
      </article>
    </section>
  </main>

  <script>
    function formatRounded(value, suffix) {
      if (typeof value !== "number" || !isFinite(value)) {
        return "--";
      }
      return value.toFixed(1) + (suffix || "");
    }

    function formatInteger(value, suffix) {
      if (typeof value !== "number" || !isFinite(value)) {
        return "--";
      }
      return String(value) + (suffix || "");
    }

    function setText(id, value) {
      document.getElementById(id).textContent = value;
    }

    function hasMeaningfulIndoorReading(data) {
      return data.indoorValid === true;
    }

    function hasMeaningfulOutdoorReading(data) {
      return data.outdoorValid === true && data.outdoorStale === false;
    }

    function setRecommendationState(headline, headlineClass, title, note, label, fillPercent) {
      var headlineNode = document.getElementById("ventingHeadline");
      headlineNode.textContent = headline;
      headlineNode.className = "hero-stat-value " + headlineClass;
      setText("ventingRecommendation", title);
      setText("ventingNote", note);
      setText("differenceLabel", label);
      document.getElementById("differenceFill").style.width = fillPercent + "%";
    }

    function showError(message) {
      var errorBox = document.getElementById("error");
      errorBox.textContent = message;
      errorBox.style.display = "block";
      setText("connectionState", "Verbindung gestört");
    }

    function clearError() {
      document.getElementById("error").style.display = "none";
      setText("connectionState", "Aktualisiert");
    }

    function updateRefreshTime() {
      var now = new Date();
      var hours = String(now.getHours()).padStart(2, "0");
      var minutes = String(now.getMinutes()).padStart(2, "0");
      setText("lastRefreshTime", hours + ":" + minutes);
    }

    function updateRecommendation(data) {
      if (!hasMeaningfulIndoorReading(data) || !hasMeaningfulOutdoorReading(data)) {
        setRecommendationState(
          "Prüfe Daten",
          "status-neutral",
          "Noch kein sinnvoller Außenwert",
          "Die Lüftungsempfehlung erscheint erst, wenn der Außensensor wieder einen plausiblen und frischen Wert liefert.",
          "Neutral",
          0
        );
        return;
      }

      var difference = data.absoluteHumidityDifferenceGm3;
      if (typeof difference !== "number" || !isFinite(difference)) {
        setRecommendationState(
          "Prüfe Daten",
          "status-neutral",
          "Noch keine belastbare Aussage",
          "Sobald beide Sensoren plausible Werte liefern, erscheint hier die Bewertung.",
          "Neutral",
          0
        );
        return;
      }

      var clampedFill = Math.min(Math.abs(difference) / 6.0, 1) * 100;

      if (difference >= 3.0) {
        setRecommendationState(
          "Lüften gut",
          "status-good",
          "Jetzt ist Lüften sinnvoll",
          "Die Außenluft ist deutlich trockener als innen und kann Feuchtigkeit wirksam abführen.",
          "Außen trockener",
          clampedFill
        );
      } else if (difference > 0.0) {
        setRecommendationState(
          "Knapp positiv",
          "status-neutral",
          "Lüften bringt etwas, aber nicht stark",
          "Die Außenluft ist trockener, der Vorteil ist im Moment aber noch relativ klein.",
          "Leichter Vorteil",
          clampedFill
        );
      } else {
        setRecommendationState(
          "Eher warten",
          "status-warn",
          "Lüften lohnt sich derzeit kaum",
          "Die Außenluft ist nicht trockener als innen. Warten auf günstigere Bedingungen ist sinnvoller.",
          "Kein Vorteil",
          clampedFill
        );
      }
    }

    function updateWeatherData(data) {
      var hasIndoorReading = hasMeaningfulIndoorReading(data);
      var hasOutdoorReading = hasMeaningfulOutdoorReading(data);

      setText("indoorTemperatureCelsius", formatRounded(data.indoorTemperatureCelsius, " °C"));
      setText("indoorHumidityPercent", formatInteger(data.indoorHumidityPercent, " %"));
      setText("indoorAbsoluteHumidityGm3", formatRounded(data.indoorAbsoluteHumidityGm3, " g/m³"));
      setText("indoorDewPointCelsius", formatRounded(data.indoorDewPointCelsius, " °C"));
      setText("outdoorTemperatureCelsius", hasOutdoorReading ? formatRounded(data.outdoorTemperatureCelsius, " °C") : "--");
      setText("outdoorHumidityPercent", hasOutdoorReading ? formatInteger(data.outdoorHumidityPercent, " %") : "--");
      setText("outdoorAbsoluteHumidityGm3", hasOutdoorReading ? formatRounded(data.outdoorAbsoluteHumidityGm3, " g/m³") : "--");
      setText("outdoorBatteryOk", hasOutdoorReading ? (data.outdoorBatteryOk ? "OK" : "Niedrig") : "--");
      setText("outdoorSecondsSinceLastReading", hasOutdoorReading ? formatInteger(data.outdoorSecondsSinceLastReading, " s") : "--");

      if (hasIndoorReading && hasOutdoorReading) {
        setText("absoluteHumidityDifferenceGm3", formatRounded(data.absoluteHumidityDifferenceGm3, " g/m³"));
        setText("heroDifference", formatRounded(data.absoluteHumidityDifferenceGm3, " g/m³"));
      } else {
        setText("absoluteHumidityDifferenceGm3", "--");
        setText("heroDifference", "--");
      }

      updateRecommendation(data);
      updateRefreshTime();
    }

    function refreshWeatherData() {
      fetch("/data.json", { cache: "no-store" })
        .then(function(response) {
          if (!response.ok) {
            throw new Error("HTTP " + response.status);
          }
          return response.json();
        })
        .then(function(data) {
          updateWeatherData(data);
          clearError();
        })
        .catch(function(error) {
          showError(error.message);
        });
    }

    refreshWeatherData();
    setInterval(refreshWeatherData, 5000);
  </script>
</body>
</html>
)=====";
