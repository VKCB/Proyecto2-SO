import serial
import time

# --- Conexion arduino---
puerto_serial = 'COM7'  
baud_rate = 9600

# Constantes físicas para el teclado
q_x_cm = 4.0
q_y_cm = 12.5
distancia_teclas = 2.1  # cm
pasos_por_cm = 114      # Ajusta según tu motor/sistema mecánico

# Mapa de teclado con posición en cm (basado en Q)
keyboard_map_cm = {
    'Q': (q_x_cm + 0 * distancia_teclas, q_y_cm + 0 * distancia_teclas),
    'W': (q_x_cm + 1 * distancia_teclas, q_y_cm + 0 * distancia_teclas),
    'E': (q_x_cm + 2 * distancia_teclas, q_y_cm + 0 * distancia_teclas),
    'R': (q_x_cm + 3 * distancia_teclas, q_y_cm + 0 * distancia_teclas),
    'T': (q_x_cm + 4 * distancia_teclas, q_y_cm + 0 * distancia_teclas),
    'Y': (q_x_cm + 5 * distancia_teclas, q_y_cm + 0 * distancia_teclas),
    'U': (q_x_cm + 6 * distancia_teclas, q_y_cm + 0 * distancia_teclas),
    'I': (q_x_cm + 7 * distancia_teclas, q_y_cm + 0 * distancia_teclas),
    'O': (q_x_cm + 8 * distancia_teclas, q_y_cm + 0 * distancia_teclas),
    'P': (q_x_cm + 9 * distancia_teclas, q_y_cm + 0 * distancia_teclas),

    'A': (q_x_cm + 0.5 * distancia_teclas, q_y_cm + 1 * distancia_teclas),
    'S': (q_x_cm + 1.5 * distancia_teclas, q_y_cm + 1 * distancia_teclas),
    'D': (q_x_cm + 2.5 * distancia_teclas, q_y_cm + 1 * distancia_teclas),
    'F': (q_x_cm + 3.5 * distancia_teclas, q_y_cm + 1 * distancia_teclas),
    'G': (q_x_cm + 4.5 * distancia_teclas, q_y_cm + 1 * distancia_teclas),
    'H': (q_x_cm + 5.5 * distancia_teclas, q_y_cm + 1 * distancia_teclas),
    'J': (q_x_cm + 6.5 * distancia_teclas, q_y_cm + 1 * distancia_teclas),
    'K': (q_x_cm + 7.5 * distancia_teclas, q_y_cm + 1 * distancia_teclas),
    'L': (q_x_cm + 8.5 * distancia_teclas, q_y_cm + 1 * distancia_teclas),

    'Z': (q_x_cm + 1 * distancia_teclas, q_y_cm + 2 * distancia_teclas),
    'X': (q_x_cm + 2 * distancia_teclas, q_y_cm + 2 * distancia_teclas),
    'C': (q_x_cm + 3 * distancia_teclas, q_y_cm + 2 * distancia_teclas),
    'V': (q_x_cm + 4 * distancia_teclas, q_y_cm + 2 * distancia_teclas),
    'B': (q_x_cm + 5 * distancia_teclas, q_y_cm + 2 * distancia_teclas),
    'N': (q_x_cm + 6 * distancia_teclas, q_y_cm + 2 * distancia_teclas),
    'M': (q_x_cm + 7 * distancia_teclas, q_y_cm + 2 * distancia_teclas),

    ' ': (q_x_cm + 4.5 * distancia_teclas, q_y_cm + 3 * distancia_teclas),  # espacio
    '\n': (q_x_cm + 9 * distancia_teclas, q_y_cm + 2 * distancia_teclas),  # enter
}

# --- Función para convertir letra a pasos ---
def letra_a_pasos(letra):
    letra = letra.upper()
    if letra not in keyboard_map_cm:
        print(f"[ERROR] Letra no reconocida o no mapeada: '{letra}'")
        return None
    x_cm, y_cm = keyboard_map_cm[letra]
    pasos_x = int(x_cm * pasos_por_cm)
    pasos_y = int(y_cm * pasos_por_cm)
    return pasos_x, pasos_y

# --- Función para enviar comando al Arduino ---
def enviar_a_arduino(ser, pasos_x, pasos_y, pasos_z=0):
    comando = f"{pasos_x},{pasos_y},{pasos_z},P\n"
    ser.write(comando.encode())
    print(f"[ENVIADO] {comando.strip()}")

# --- Función principal para escribir texto ---
def escribir_texto(ser, texto, delay=0.7):
    for letra in texto:
        pasos = letra_a_pasos(letra)
        if pasos:
            # Mueve a la posición (X, Y)
            enviar_a_arduino(ser, pasos[0], pasos[1], 0)  # Z arriba
            time.sleep(0.3)
            # Baja Z para presionar
            enviar_a_arduino(ser, pasos[0], pasos[1], 1)  # Z abajo
            time.sleep(0.3)
            # Sube Z
            enviar_a_arduino(ser, pasos[0], pasos[1], 0)  # Z arriba
            time.sleep(delay)  # espera a que Arduino haga el movimiento y presione

# --- Programa principal ---
if __name__ == "__main__":
    try:
        ser = serial.Serial(puerto_serial, baud_rate, timeout=1)
        time.sleep(2)  # espera inicial para que Arduino resetee
        print("[INFO] Conexión serial establecida.")

        texto_a_escribir = "hola mundo  esto es una prueba de que escriba en el teclado\n"
        escribir_texto(ser, texto_a_escribir)

        ser.close()
        print("[INFO] Comunicación finalizada.")

    except serial.SerialException:
        print(f"[ERROR] No se pudo abrir el puerto serial {puerto_serial}")
