# Genera los diagramas PNG de la documentacion:
#   flujo_programa.png       diagrama de flujo del software
#   maquina_de_estados.png   maquina de estados del relato
#   ../gestion/Gantt_Proyecto.png   diagrama de Gantt
# Requiere Pillow:  python3 -m venv /tmp/v && /tmp/v/bin/pip install pillow && /tmp/v/bin/python generar_diagramas.py
# Los textos y las fechas se editan aqui; las versiones Mermaid (.mmd) son equivalentes editables.
from PIL import Image, ImageDraw, ImageFont
import os, datetime as dt

AQUI = os.path.dirname(os.path.abspath(__file__))
F = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
FB = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
def font(n, b=False): return ImageFont.truetype(FB if b else F, n)

BG = (250, 250, 247); INK = (30, 34, 44); GRIS = (120, 126, 140)
AZUL = (52, 101, 164); VERDE = (60, 140, 90); NARANJA = (214, 120, 40); ROJO = (180, 58, 52); MORADO = (110, 80, 160); AMARILLO = (232, 196, 80)

def wrap(d, txt, fnt, ancho):
    out = []
    for parrafo in txt.split("\n"):
        linea = ""
        for w in parrafo.split():
            t = (linea + " " + w).strip()
            if d.textlength(t, font=fnt) <= ancho: linea = t
            else: out.append(linea); linea = w
        out.append(linea)
    return out

def caja(d, x, y, w, h, txt, fill, borde, fnt=None, color=INK, radio=16, negrita=False):
    fnt = fnt or font(22, negrita)
    d.rounded_rectangle((x, y, x + w, y + h), radius=radio, fill=fill, outline=borde, width=3)
    ls = wrap(d, txt, fnt, w - 28); alto = fnt.size + 6
    yy = y + (h - alto * len(ls)) / 2
    for l in ls:
        d.text((x + w / 2 - d.textlength(l, font=fnt) / 2, yy), l, font=fnt, fill=color); yy += alto

def flecha(d, p1, p2, color=INK, ancho=3, texto=None, fnt=None, desp=(8, -26)):
    d.line([p1, p2], fill=color, width=ancho)
    import math
    a = math.atan2(p2[1] - p1[1], p2[0] - p1[0]); L = 14
    d.polygon([p2, (p2[0] - L * math.cos(a - .45), p2[1] - L * math.sin(a - .45)), (p2[0] - L * math.cos(a + .45), p2[1] - L * math.sin(a + .45))], fill=color)
    if texto:
        fnt = fnt or font(18)
        d.text(((p1[0] + p2[0]) / 2 + desp[0], (p1[1] + p2[1]) / 2 + desp[1]), texto, font=fnt, fill=GRIS)

def polilinea(d, pts, color=INK, ancho=3):
    for a, b in zip(pts[:-2], pts[1:-1]): d.line([a, b], fill=color, width=ancho)
    flecha(d, pts[-2], pts[-1], color, ancho)

# ================================================= 1. FLUJO DEL PROGRAMA =================================================
def flujo():
    W, H = 1700, 2700
    im = Image.new("RGB", (W, H), BG); d = ImageDraw.Draw(im)
    d.text((W / 2 - 360, 24), "Diagrama de flujo del software (main.cpp)", font=font(34, True), fill=INK)
    cx, bw, bh = 440, 760, 100
    pasos = [
        ("INICIO", None, AMARILLO),
        ("1. Iniciar GLFW, crear la ventana de 1280 x 720 y cargar OpenGL con GLAD", None, (222, 232, 246)),
        ("2. Compilar los shaders: lighting.vs / lighting.frag (escena) y hud.vs / hud.frag (texto)", None, (222, 232, 246)),
        ("3. Cargar modelos con Assimp: plaza, mallas de pose (CUAU, SENTINEL, railgun, MAC, aspa) y rigs por partes (CUAU 16, SENTINEL 13, obreros UNAM e IPN)", None, (222, 232, 246)),
        ("4. Crear el HUD (cuadro 2D), leer opciones de linea de comandos y ajustes_escenas.txt, e IniciarEscena(0)", None, (222, 232, 246)),
    ]
    y = 90
    ys = []
    for t, _, col in pasos:
        h = 130 if t.startswith("3.") or t.startswith("4.") else bh
        if t == "INICIO": h = 70
        caja(d, cx, y, bw, h, t, col, AZUL, negrita=(t == "INICIO")); ys.append((y, h)); y += h + 46
    for i in range(len(ys) - 1):
        flecha(d, (cx + bw / 2, ys[i][0] + ys[i][1]), (cx + bw / 2, ys[i + 1][0]))
    # ciclo
    ini_ciclo = y
    d.rounded_rectangle((cx - 70, ini_ciclo - 20, cx + bw + 70, ini_ciclo + 1160), radius=24, outline=VERDE, width=4)
    d.text((cx - 50, ini_ciclo - 14), "CICLO PRINCIPAL (un cuadro)", font=font(20, True), fill=VERDE)
    flecha(d, (cx + bw / 2, ys[-1][0] + ys[-1][1]), (cx + bw / 2, ini_ciclo + 28))
    ciclo = [
        "5. Calcular deltaTime y procesar eventos de teclado y raton (KeyCallback, DoMovement, MouseCallback)",
        "6. Animation() y ActualizarRelato(dt): avanzar la fase de la escena y calcular posiciones y poses de los actores",
        "7. Calcular las matrices de los SENTINEL (barrido) y la posicion de la luz de destello",
        "8. Enviar uniformes (camara, luces, emissive, brillo) y dibujar la plaza",
        "9. DibujarRelato(): obreros, CUAU, MAC y SENTINEL de la escena actual",
        "10. DibujarHUDRelato(): recuadro de texto o de accion, con mezcla alfa",
    ]
    yy = ini_ciclo + 28; pos = []
    for t in ciclo:
        caja(d, cx, yy, bw, 100, t, (226, 240, 230), VERDE); pos.append(yy); yy += 100 + 44
    for i in range(len(pos) - 1): flecha(d, (cx + bw / 2, pos[i] + 100), (cx + bw / 2, pos[i + 1]))
    # decision --foto
    dy = yy
    d.polygon([(cx + bw / 2, dy), (cx + bw, dy + 55), (cx + bw / 2, dy + 110), (cx, dy + 55)], fill=(252, 238, 214), outline=NARANJA)
    d.text((cx + bw / 2 - d.textlength("11. ¿opcion --foto?", font=font(21)) / 2, dy + 42), "11. ¿opcion --foto?", font=font(21), fill=INK)
    flecha(d, (cx + bw / 2, pos[-1] + 100), (cx + bw / 2, dy))
    sx = cx + bw + 110
    caja(d, sx, dy + 15, 320, 80, "Guardar captura PNG y salir del ciclo", (252, 238, 214), NARANJA, font(19))
    flecha(d, (cx + bw, dy + 55), (sx, dy + 55), texto="si", desp=(-20, -28))
    # intercambio de buffers
    by = dy + 160
    caja(d, cx, by, bw, 80, "12. Intercambiar buffers (glfwSwapBuffers)", (226, 240, 230), VERDE)
    flecha(d, (cx + bw / 2, dy + 110), (cx + bw / 2, by), texto="no", desp=(10, -20))
    # decision cerrar
    cy = by + 125
    d.polygon([(cx + bw / 2, cy), (cx + bw, cy + 75), (cx + bw / 2, cy + 150), (cx, cy + 75)], fill=(252, 238, 214), outline=NARANJA)
    for i, l in enumerate(["12b. ¿Cerrar?", "(Esc, ventana o Enter final)"]):
        d.text((cx + bw / 2 - d.textlength(l, font=font(21, i == 0)) / 2, cy + 52 + i * 30), l, font=font(21, i == 0), fill=INK)
    flecha(d, (cx + bw / 2, by + 80), (cx + bw / 2, cy))
    # regreso al ciclo
    polilinea(d, [(cx, cy + 75), (cx - 110, cy + 75), (cx - 110, ini_ciclo + 78), (cx, ini_ciclo + 78)], color=VERDE)
    d.text((cx - 330, cy + 36), "no: siguiente cuadro", font=font(19), fill=VERDE)
    # fin
    fy = cy + 240
    caja(d, cx + 130, fy, 500, 90, "Guardar ajustes pendientes, liberar GLFW\nFIN", AMARILLO, AZUL, font(20, True))
    flecha(d, (cx + bw / 2, cy + 150), (cx + bw / 2, fy), texto="si", desp=(10, -20))
    im.crop((0, 0, W, fy + 140)).save(os.path.join(AQUI, "flujo_programa.png"))

# ================================================= 2. MAQUINA DE ESTADOS =================================================
def maquina():
    W, H = 2100, 1500
    im = Image.new("RGB", (W, H), BG); d = ImageDraw.Draw(im)
    d.text((60, 28), "Maquina de estados del relato", font=font(36, True), fill=INK)
    d.text((60, 78), "Cada escena recorre las mismas fases; el estado se guarda en la variable fase (enum Fase) y se actualiza en ActualizarRelato(dt).", font=font(21), fill=GRIS)
    est = [("TEXTO", "F_TEXTO", "Se muestra la historia en pantalla (hasta 3 paginas). El teclado solo acepta Enter.", AZUL),
           ("CAMARA", "F_CAMARA", "La camara viaja con lookAt: posicion y punto de mira se interpolan con smoothstep. Sin entrada.", MORADO),
           ("ESPERA", "F_ESPERA", "Recuadro de accion: se espera la tecla de la escena (Espacio, o flechas en la 4).", NARANJA),
           ("ANIMA", "F_ANIMA", "Corre la animacion de la escena durante DurAnim(escena) segundos. El teclado se ignora.", VERDE),
           ("FIN", "F_FIN", "«Presiona ENTER para continuar». Con Enter se pasa a la escena siguiente.", ROJO)]
    bw, bh, gap, x0, y0 = 290, 190, 130, 60, 190
    xs = []
    for i, (n, c, desc, col) in enumerate(est):
        x = x0 + i * (bw + gap); xs.append(x)
        d.rounded_rectangle((x, y0, x + bw, y0 + bh), radius=22, fill=(255, 255, 255), outline=col, width=5)
        d.rounded_rectangle((x, y0, x + bw, y0 + 54), radius=22, fill=col)
        d.rectangle((x, y0 + 30, x + bw, y0 + 54), fill=col)
        d.text((x + 20, y0 + 10), n, font=font(28, True), fill=(255, 255, 255)); d.text((x + bw - 15 - d.textlength(c, font=font(17)), y0 + 18), c, font=font(17), fill=(255, 255, 255))
        for j, l in enumerate(wrap(d, desc, font(18), bw - 30)): d.text((x + 15, y0 + 66 + j * 24), l, font=font(18), fill=INK)
    etiquetas = ["Enter en la\nultima pagina", "la camara\nllega al destino", "tecla de\nla escena", "termina\nDurAnim()"]
    for i in range(4):
        flecha(d, (xs[i] + bw, y0 + bh / 2), (xs[i + 1], y0 + bh / 2), ancho=4)
        for j, l in enumerate(etiquetas[i].split("\n")): d.text((xs[i] + bw + 8, y0 + bh / 2 - 56 + j * 21), l, font=font(16), fill=GRIS)
    # FIN -> siguiente escena
    flecha(d, (xs[4] + bw / 2, y0 + bh), (xs[4] + bw / 2, y0 + bh + 70), ancho=4)
    d.text((xs[4] - 120, y0 + bh + 76), "Enter: IniciarEscena(i + 1)\n(escena 9: Enter cierra el programa)", font=font(18), fill=INK)
    # casos especiales
    yb = 520
    d.text((60, yb - 40), "Casos especiales", font=font(26, True), fill=INK)
    esp = [(AZUL, "Escenas 0 y 9: textoDespues", "El orden se invierte: CAMARA → TEXTO → FIN. La historia aparece cuando la camara ya llego."),
           (NARANJA, "Escena 1: tres martillazos", "ESPERA se repite: cada Espacio da un martillazo (golpesObr = 0 → 3). Con el tercero, el siguiente Espacio pasa a ANIMA (CUAU despierta)."),
           (VERDE, "Escena 2: automatica", "Al terminar CAMARA pasa directo a ANIMA, sin esperar tecla (los SENTINEL salen del templo). Luego FIN."),
           (ROJO, "Escena 4: tres esquives", "ANIMA vuelve a ESPERA hasta completar la secuencia ← → ← (pasoEsq = 0 → 3). Una flecha equivocada solo muestra el aviso «Tecla incorrecta».")]
    for i, (col, tit, txt) in enumerate(esp):
        x = 60 + (i % 2) * 1000; y = yb + (i // 2) * 175
        d.rounded_rectangle((x, y, x + 960, y + 150), radius=18, fill=(255, 255, 255), outline=col, width=4)
        d.rectangle((x, y + 18, x + 14, y + 132), fill=col)
        d.text((x + 34, y + 14), tit, font=font(23, True), fill=col)
        for j, l in enumerate(wrap(d, txt, font(19), 900)): d.text((x + 34, y + 54 + j * 26), l, font=font(19), fill=INK)
    # tira de escenas
    ys = 920
    d.text((60, ys - 44), "Las 10 escenas", font=font(26, True), fill=INK)
    esc = [("0", "Introduccion", AZUL), ("1", "Obreros", AMARILLO), ("2", "SENTINEL salen", ROJO), ("3", "Levanta la MAC", VERDE), ("4", "Sprint", VERDE),
           ("5", "Azote", VERDE), ("6", "Punetazo", VERDE), ("7", "Azote final", VERDE), ("8", "Victoria", VERDE), ("9", "Lapida", AZUL)]
    cw = 190
    for i, (n, t, col) in enumerate(esc):
        x = 60 + i * (cw + 10)
        d.rounded_rectangle((x, ys, x + cw, ys + 110), radius=14, fill=(255, 255, 255), outline=col, width=4)
        d.text((x + 12, ys + 8), n, font=font(30, True), fill=col)
        for j, l in enumerate(wrap(d, t, font(19), cw - 24)): d.text((x + 12, ys + 52 + j * 24), l, font=font(19), fill=INK)
        if i < 9: flecha(d, (x + cw, ys + 55), (x + cw + 10, ys + 55), ancho=3)
    d.text((60, ys + 124), "Zona prehispanica: escenas 1 y 3  ·  Templo de Santiago: escena 2  ·  Plaza abierta: escenas 4 a 8  ·  Zona contemporanea (lapida): escena 9", font=font(19), fill=GRIS)
    # leyenda de variables
    d.rounded_rectangle((60, 1100, 2040, 1440), radius=18, fill=(255, 255, 255), outline=GRIS, width=2)
    d.text((84, 1116), "Variables de estado (main.cpp)", font=font(24, True), fill=INK)
    vars_ = ["escena (0 a 9) · fase (Fase) · pagina (pagina de texto actual) · tFase (tiempo en la fase) · tAnim (tiempo de la animacion)",
             "golpesObr (martillazos, escena 1) · pasoEsq (esquives hechos, escena 4) · tGolpe (tiempo desde el ultimo martillazo) · avisoT (aviso de tecla incorrecta)",
             "camPos, camMira (camara actual) · camDesde, miraDesde (origen del viaje) · ESC[escena].cam, .mira, .durCam (destino y duracion)",
             "cuauVisible, macVisible, obrerosVisible, sentVis[3] (que actores se dibujan) · cuauPosR, cuauYawR, sentPos[3], sentCaida[3] (donde y como)",
             "Funciones: IniciarEscena · ActualizarRelato · ProcesarTeclaRelato · TrasTexto · TrasCamara · IrAEscena (modo desarrollador)"]
    for i, l in enumerate(vars_):
        for j, ll in enumerate(wrap(d, l, font(19), 1900)): d.text((84, 1160 + i * 52 + j * 24), ll, font=font(19), fill=INK)
    im.save(os.path.join(AQUI, "maquina_de_estados.png"))

# ================================================= 3. GANTT =================================================
def gantt():
    D = dt.date
    ini, fin = D(2026, 9, 28), D(2026, 11, 10)
    dias = (fin - ini).days
    W = 2100; mx = 640; ancho = W - mx - 40; px = ancho / dias
    filas = [("Prototipo", "hito", D(2026, 9, 29), D(2026, 9, 29), AZUL, "Entrega parcial 29 sep"),
             ("Modelado", "barra", D(2026, 9, 29), D(2026, 10, 13), AZUL, "Entrega parcial 13 oct"),
             ("Texturizado, color e iluminacion", "barra", D(2026, 10, 13), D(2026, 10, 27), AZUL, "Entrega parcial 27 oct"),
             ("Animacion", "barra", D(2026, 10, 27), D(2026, 11, 5), AZUL, "Entrega parcial 5 nov"),
             ("Integracion en OpenGL (proyecto final)", "barra", D(2026, 11, 6), D(2026, 11, 9), VERDE, "6 al 9 nov"),
             ("Relato interactivo: maquina de estados y HUD", "barra", D(2026, 11, 8), D(2026, 11, 9), VERDE, "8 y 9 nov"),
             ("Documentacion (manuales, diagramas, Gantt)", "barra", D(2026, 11, 8), D(2026, 11, 9), NARANJA, "8 y 9 nov"),
             ("Video explicativo y exposicion", "barra", D(2026, 11, 9), D(2026, 11, 9), NARANJA, "9 nov"),
             ("Entrega en Classroom", "hito", D(2026, 11, 9), D(2026, 11, 9), ROJO, "9 nov, 11:59 p. m.")]
    top = 170; fh = 64
    H = top + fh * len(filas) + 380
    im = Image.new("RGB", (W, H), BG); d = ImageDraw.Draw(im)
    d.text((40, 24), "Diagrama de Gantt — T68 Sci-Fi Tribute", font=font(34, True), fill=INK)
    d.text((40, 76), "Fases acotadas por las cuatro entregas parciales de la clase de teoria y por la entrega final. Las barras son ventanas de entrega, no horas trabajadas.", font=font(20), fill=GRIS)
    # ejes
    x_de = lambda f: mx + (f - ini).days * px
    f = ini
    while f <= fin:
        x = x_de(f)
        if f.weekday() == 0:
            d.line([(x, top - 12), (x, top + fh * len(filas))], fill=(205, 205, 200), width=2)
            d.text((x + 3, top - 40), f.strftime("%d/%m"), font=font(18), fill=GRIS)
        f += dt.timedelta(days=1)
    for i, (nom, tipo, a, b, col, nota) in enumerate(filas):
        y = top + i * fh
        if i % 2 == 0: d.rectangle((20, y, W - 20, y + fh), fill=(243, 243, 238))
        d.text((40, y + 18), nom, font=font(21), fill=INK)
        xa, xb = x_de(a), x_de(b + dt.timedelta(days=1)) if tipo == "barra" else x_de(a)
        if tipo == "hito":
            cx = x_de(a) + px / 2
            d.polygon([(cx, y + 12), (cx + 18, y + fh / 2), (cx, y + fh - 12), (cx - 18, y + fh / 2)], fill=col)
            if cx > W - 450: d.text((cx - 28 - d.textlength(nota, font=font(19, True)), y + 19), nota, font=font(19, True), fill=col)
            else: d.text((cx + 28, y + 19), nota, font=font(19, True), fill=col)
        else:
            d.rounded_rectangle((xa, y + 12, max(xb, xa + 14), y + fh - 12), radius=8, fill=col)
            if xb > W - 450: d.text((xa - 14 - d.textlength(nota, font=font(19)), y + 19), nota, font=font(19), fill=col)
            else: d.text((max(xb, xa + 14) + 12, y + 19), nota, font=font(19), fill=col)
    # commits reales
    cy = top + fh * len(filas) + 40
    d.text((40, cy), "Historial real de commits (git):", font=font(22, True), fill=INK)
    commits = [(D(2026, 11, 6), "6 nov 08:44 · estructura inicial"), (D(2026, 11, 8), "8 nov · 3 commits (13:11, 17:23 docs, 22:58)"), (D(2026, 11, 9), "9 nov · 00:50 y 00:55")]
    yl = cy + 40
    d.line([(mx, yl + 25), (W - 40, yl + 25)], fill=GRIS, width=2)
    for fch, txt in commits:
        x = x_de(fch) + px / 2
        d.ellipse((x - 9, yl + 16, x + 9, yl + 34), fill=ROJO)
    for k, (fch, txt) in enumerate([(D(2026, 11, 6), "6 nov: 1 commit (08:44)"), (D(2026, 11, 8), "8 nov: 3 commits (13:11, 17:23, 22:58)"), (D(2026, 11, 9), "9 nov: 2 commits (00:50, 00:55)")]):
        x = x_de(fch) + px / 2; yy = yl + 44 + k * 26
        d.line([(x, yl + 34), (x, yy + 4)], fill=(200, 160, 160), width=1)
        d.text((x - 12 - d.textlength(txt, font=font(17)), yy), txt, font=font(17), fill=GRIS)
    d.text((40, yl + 142), "Azul: entregas parciales de teoria · Verde: proyecto final en OpenGL · Naranja: documentacion y presentacion · Rojo: entrega.", font=font(19), fill=GRIS)
    d.text((40, yl + 172), "El trabajo de integracion en OpenGL y el relato interactivo se concentro en los ultimos dias; el modelado, texturizado y animacion previos corresponden a las entregas de teoria.", font=font(19), fill=GRIS)
    out = os.path.join(AQUI, "..", "gestion", "Gantt_Proyecto.png")
    im.crop((0, 0, W, yl + 215)).save(out)

if __name__ == "__main__":
    flujo(); maquina(); gantt(); print("diagramas generados")
