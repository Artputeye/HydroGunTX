let lastData = {
    updown: 0,
    leftright: 0,
    rssi: 0,
    loss: 0,
    fire: false,
    espnow: false
};

function isChanged(key, value) {
    if (lastData[key] !== value) {
        lastData[key] = value;
        return true;
    }
    return false;
}

var websocket;

function initWebSocket() {
    websocket = new WebSocket(`ws://${window.location.hostname}/ws`);

    websocket.onmessage = (event) => {
        try {
            let json = JSON.parse(event.data);

            if ("fire" in json)
                updateStatus("fire", json.fire, "ON", "OFF");

            if ("espnow" in json)
                updateStatus("status", json.espnow, "Connected", "Unconnect");

            if ("updown" in json) lastData.updown = json.updown;
            if ("leftright" in json) lastData.leftright = json.leftright;
            document.getElementById("updownValue").innerText = lastData.updown;
            document.getElementById("leftrightValue").innerText = lastData.leftright;
            console.log("Joystick Data - UpDown:", json.updown, "LeftRight:", json.leftright);


            if ("rssi" in json && isChanged("rssi", json.rssi))
                document.getElementById("signalValue").innerText = json.rssi + " dBm";

            if ("loss" in json && isChanged("loss", json.loss))
                document.getElementById("lossValue").innerText = json.loss + "%";

        } catch (e) {
            console.error("JSON error:", e);
        }
    };
}

window.addEventListener("load", initWebSocket);


/**
 * ฟังก์ชันควบคุมสถานะไฟและข้อความ
 * @param {string} prefix - ชื่อนำหน้าของ ID (เช่น 'status' หรือ 'fire')
 * @param {boolean} state - true คือ ON/Connected, false คือ OFF/Unconnect
 * @param {string} onText - ข้อความเมื่อสถานะเป็น true
 * @param {string} offText - ข้อความเมื่อสถานะเป็น false
 */
function updateStatus(prefix, state, onText, offText) {
    const lamp = document.getElementById(prefix + "Lamp");
    const text = document.getElementById(prefix + "Text");

    if (state) {
        lamp.classList.replace("lamp-off", "lamp-on");
        text.innerText = onText;
    } else {
        lamp.classList.replace("lamp-on", "lamp-off");
        text.innerText = offText;
    }
}

// --- วิธีใช้งาน ---

// เมื่อต้องการเปลี่ยนสถานะเชื่อมต่อ
// updateStatus("status", true, "Connected", "Unconnect");

// เมื่อต้องการสั่ง Fire (ยิง)
// updateStatus("fire", true, "ON", "OFF");

window.addEventListener("load", () => {
    // ตั้งค่าเริ่มต้นตอนโหลดหน้าเว็บ
    updateStatus("status", false, "Connected", "Unconnect");
    updateStatus("fire", false, "ON", "OFF");
});





