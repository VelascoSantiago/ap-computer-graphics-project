# Genera los PNG del HUD (texto de la historia y mensajes de accion) en esta misma carpeta.
# Los PNG se dibujan en main.cpp como cuadros 2D (shader hud.vs/.frag), sin librerias de fuentes.
# Requiere Pillow:   python3 -m venv /tmp/v && /tmp/v/bin/pip install pillow && /tmp/v/bin/python generar_textos.py
# Cada imagen se genera al DOBLE de resolucion (texto nitido en pantallas HiDPI); main.cpp la dibuja a la mitad.
from PIL import Image, ImageDraw, ImageFont
import os

AQUI = os.path.dirname(os.path.abspath(__file__))
FUENTE = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
FUENTE_B = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
K = 2   # factor de resolucion

# ----------------------------- textos de la historia (Enter avanza) -----------------------------
PAGINAS = {
    "t0a": ["2 de octubre de 1968", "Plaza de las Tres Culturas, Tlatelolco", "Ciudad de México"],
    "t0b": ["Aquella tarde, una concentración estudiantil fue reprimida con violencia.",
            "Muchas personas murieron o resultaron heridas.", "Su memoria sigue viva."],
    "t0c": ["Esta recreación es un homenaje, hecho con respeto.",
            "Es una ficción: dos robots, CUAU-68 y los SENTINEL,",
            "representan la valentía de las y los estudiantes", "y la fuerza que quiso callarla."],
    "t1a": ["Zona prehispánica. Desde nuestras raíces,", "resistimos y defendemos lo nuestro."],
    "t1b": ["Aquí nace CUAU-68: Coraza Unificada de Acción Universitaria.",
            "Su nombre recuerda a Cuauhtémoc, el último tlatoani,",
            "que defendió Tenochtitlan hasta caer en Tlatelolco, en 1521."],
    "t1c": ["Estudiantes de la UNAM y del Politécnico lo construyen",
            "con piezas tomadas de los SENTINEL MX."],
    "t2a": ["Del templo colonial salen los SENTINEL MX,", "los robots del gobierno: el ejército."],
    "t2b": ["La iglesia nos recuerda la invasión y la opresión.",
            "Avanzan en fila, con sus railguns en la mano."],
    "t3a": ["CUAU-68 empuña la MAC:", "Mecanismo Acelerador de Cadena."],
    "t3b": ["Sus hojas son de obsidiana, en memoria del macuahuitl,", "el arma de los antiguos mexicas."],
    "t4a": ["Los SENTINEL abren fuego.", "CUAU-68 corre y esquiva."],
    "t5a": ["CUAU-68 contraataca:", "la MAC cae sobre un SENTINEL."],
    "t6a": ["Un puñetazo derriba", "a otro SENTINEL."],
    "t7a": ["El último SENTINEL", "cae bajo la MAC."],
    "t8a": ["La plaza queda en silencio."],
    "t9a": ["2 de octubre de 1968"],
    "t9b": ["Para quienes no volvieron a casa.", "Para quienes siguen buscando verdad y justicia."],
    "t9c": ["Esta lápida está en la zona contemporánea:", "no olvidamos, ni ahora ni en el futuro.", "2 de octubre no se olvida."],
}

# ----------------------------- mensajes de accion (prompts) -----------------------------
PROMPTS = {
    "p_enter":   "Presiona ENTER para continuar",
    "p1_m0":     "Presiona ESPACIO para dar un martillazo   (0/3)",
    "p1_m1":     "Presiona ESPACIO para dar un martillazo   (1/3)",
    "p1_m2":     "Presiona ESPACIO para dar un martillazo   (2/3)",
    "p1_des":    "Presiona ESPACIO para despertar a CUAU-68",
    "p3":        "Presiona ESPACIO para levantar la MAC",
    "p4_a":      "Esquiva: presiona  ←   (1/3)",
    "p4_b":      "Esquiva: presiona  →   (2/3)",
    "p4_c":      "Esquiva: presiona  ←   (3/3)",
    "p4_mal":    "Tecla incorrecta",
    "p5":        "Presiona ESPACIO para golpear con la MAC",
    "p6":        "Presiona ESPACIO para dar el golpe",
    "p7":        "Presiona ESPACIO para el golpe final",
    "p8":        "Presiona ESPACIO para alzar la MAC",
    "p_fin":     "FIN   ·   ENTER para cerrar   ·   ESC para salir",
}

def panel(lineas, tam, ancho_min, pista=False, color=(245, 245, 245), acento=(255, 214, 102)):
    f = ImageFont.truetype(FUENTE, tam * K)
    pad, inter = 34 * K, 12 * K
    sizes = [f.getbbox(l) for l in lineas]
    w = max(max(b[2] - b[0] for b in sizes) + 2 * pad, ancho_min * K)
    alto_l = f.getbbox("Ágjy")[3] - f.getbbox("Ágjy")[1]
    h = 2 * pad + len(lineas) * alto_l + (len(lineas) - 1) * inter + (22 * K if pista else 0)
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle((0, 0, w - 1, h - 1), radius=18 * K, fill=(8, 10, 18, 215), outline=acento + (200,), width=2 * K)
    y = pad
    for l in lineas:
        bb = f.getbbox(l)
        d.text(((w - (bb[2] - bb[0])) // 2 - bb[0], y - f.getbbox("Á")[1] + (f.getbbox("Á")[1])), l, font=f, fill=color)
        y += alto_l + inter
    if pista:
        fp = ImageFont.truetype(FUENTE, 18 * K)
        d.text((w - pad - fp.getlength("ENTER ▸"), h - pad // 2 - 20 * K), "ENTER ▸", font=fp, fill=(170, 170, 170, 255))
    return img

def prompt(texto, tam=30, color=(255, 214, 102)):
    f = ImageFont.truetype(FUENTE_B, tam * K)
    bb = f.getbbox(texto)
    pad = 26 * K
    w, h = (bb[2] - bb[0]) + 2 * pad, 64 * K
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle((0, 0, w - 1, h - 1), radius=16 * K, fill=(8, 10, 18, 215), outline=color + (210,), width=2 * K)
    d.text((pad - bb[0], (h - (bb[3] - bb[1])) // 2 - bb[1]), texto, font=f, fill=color)
    return img

if __name__ == "__main__":
    for nombre, lineas in PAGINAS.items():
        grande = nombre in ("t9a",)
        panel(lineas, 46 if grande else 32, 1000, pista=True).save(os.path.join(AQUI, nombre + ".png"))
    for nombre, texto in PROMPTS.items():
        color = (255, 120, 110) if nombre == "p4_mal" else (255, 214, 102)
        prompt(texto, color=color).save(os.path.join(AQUI, nombre + ".png"))
    print("PNG generados en", AQUI, ":", len(PAGINAS) + len(PROMPTS))
