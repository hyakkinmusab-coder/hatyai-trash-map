<!DOCTYPE html>
<html lang="th">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    
    <title>หาดใหญ่สะอาด - ระบบแจ้งขยะล้นและแผนที่ถังขยะหาดใหญ่</title>
    <meta name="description" content="ร่วมสร้างเมืองหาดใหญ่ให้น่าอยู่ แจ้งถังขยะเต็ม จุดทิ้งขยะเถื่อน และดูแผนที่ถังขยะทั่วเมืองหาดใหญ่ได้ที่นี่">
    <meta name="keywords" content="แจ้งขยะ, หาดใหญ่, ถังขยะ, เทศบาลนครหาดใหญ่, แผนที่ขยะ">

    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" />
    <link href="https://fonts.googleapis.com/css2?family=Sarabun:wght@300;400;600;700&display=swap" rel="stylesheet">
    
    <style>
        body, html { margin: 0; padding: 0; font-family: 'Sarabun', sans-serif; height: 100%; overflow: hidden; background: #f0f2f5; }
        #map { height: 100%; width: 100%; }
        
        /* ดีไซน์ UI กระจกฝ้า (Glassmorphism) */
        .bottom-panel { 
            position: fixed; bottom: 15px; left: 15px; right: 15px; 
            background: rgba(255, 255, 255, 0.9); z-index: 1000; 
            padding: 20px; border-radius: 20px; 
            box-shadow: 0 10px 30px rgba(0,0,0,0.15);
            backdrop-filter: blur(10px); -webkit-backdrop-filter: blur(10px);
            max-height: 45vh; overflow-y: auto;
            border: 1px solid rgba(255,255,255,0.3);
        }

        .header-section { border-bottom: 2px solid #28a745; margin-bottom: 15px; padding-bottom: 10px; display: flex; justify-content: space-between; align-items: center; }
        .header-section h2 { margin: 0; color: #1e7e34; font-size: 1.4em; font-weight: 700; }

        .btn { border: none; padding: 12px; border-radius: 12px; font-weight: 600; cursor: pointer; transition: 0.3s; font-family: 'Sarabun', sans-serif; width: 100%; margin-bottom: 10px; font-size: 1em; }
        .btn-gps { background: #1a73e8; color: white; box-shadow: 0 4px 12px rgba(26,115,232,0.3); }
        .btn-report { background: #d32f2f; color: white; margin-top: 10px; font-size: 1.1em; }
        .btn-outline { background: white; border: 1px solid #ddd; color: #555; }
        .btn-outline.active { background: #28a745; color: white; border-color: #28a745; }

        .filter-row { display: grid; grid-template-columns: repeat(4, 1fr); gap: 5px; margin-bottom: 15px; }
        
        /* ปุ่มสลับโหมดแผนที่ */
        .map-type-btn { position: fixed; top: 15px; right: 15px; z-index: 1000; background: white; border: none; padding: 8px 15px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.2); cursor: pointer; font-family: 'Sarabun', sans-serif; font-weight: 600; }

        .report-form { background: #f8f9fa; padding: 15px; border-radius: 15px; border: 1px solid #eee; }
    </style>
</head>
<body>

    <button class="map-type-btn" onclick="switchMap()">🛰️ สลับโหมดดาวเทียม</button>
    
    <div id="map"></div>

    <div class="bottom-panel">
        <div class="header-section">
            <h2>📍 หาดใหญ่สะอาด</h2>
            <button onclick="toggleAdmin()" style="background:none; border:none; color:#888; cursor:pointer; font-size: 0.8em; text-decoration: underline;">จัดการข้อมูล</button>
        </div>
        
        <button class="btn btn-gps" onclick="findMyLocation()">🔵 ระบุพิกัดปัจจุบันของคุณ</button>

        <div class="filter-row">
            <button class="btn btn-outline active" onclick="filterMap('ทั้งหมด', this)">ทั้งหมด</button>
            <button class="btn btn-outline" onclick="filterMap('ทั่วไป', this)">ทั่วไป</button>
            <button class="btn btn-outline" onclick="filterMap('รีไซเคิล', this)">รีไซเคิล</button>
            <button class="btn btn-outline" onclick="filterMap('อันตราย', this)">อันตราย</button>
        </div>

        <div class="report-form">
            <label style="font-weight: 700; color: #444; font-size: 0.9em;">ต้องการแจ้งปัญหา:</label>
            <select id="issueType" class="btn btn-outline" style="text-align:left; margin-top:5px; margin-bottom:0;">
                <option value="ขยะล้นถัง">ขยะล้นถัง / ส่งกลิ่นเหม็น</option>
                <option value="ถังขยะชำรุด">ถังขยะชำรุด / เสียหาย</option>
                <option value="ทิ้งที่ห้ามทิ้ง">พบจุดทิ้งขยะเถื่อน</option>
            </select>
            <button class="btn btn-report" id="reportBtn" onclick="sendReport()">🚀 ส่งรายงานเข้าสู่ระบบ</button>
        </div>
    </div>

    <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
    <script>
        // --- 🔗 นำลิงก์ของคุณมาวางตรงนี้ ---
        const SCRIPT_URL = "https://script.google.com/macros/s/AKfycbxcO3FT6R0YCo5qr6SJJVObi0K6G6F88D_IxD00eIZGA-S8qMFvwMQNp5B7WvAHewPS2w/exec"; 
        const SHEET_URL = "https://docs.google.com/spreadsheets/d/1K8iiW-yG929fjaDvFr_ZO6Vs8T6YrkJCDg3AjNdyx4g/edit?gid=0#gid=0";

        // ตั้งค่าแผนที่
        const streetMap = L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png');
        const satelliteMap = L.tileLayer('https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}');

        const map = L.map('map', { zoomControl: false, layers: [streetMap] }).setView([7.0104, 100.4747], 14);
        let isSatellite = false;

        function switchMap() {
            if (isSatellite) { map.removeLayer(satelliteMap); map.addLayer(streetMap); } 
            else { map.removeLayer(streetMap); map.addLayer(satelliteMap); }
            isSatellite = !isSatellite;
        }

        let markerLayer = L.layerGroup().addTo(map);
        let userMarker;
        let rawData = [];

        // โหลดข้อมูลและปักหมุดทั้งหมด
        async function loadData() {
            if(SCRIPT_URL.includes("วาง_WEB_APP")) return;
            try {
                const res = await fetch(SCRIPT_URL);
                rawData = await res.json();
                renderMarkers('ทั้งหมด');
                if (rawData.length > 0) {
                    const group = new L.featureGroup(markerLayer.getLayers());
                    map.fitBounds(group.getBounds().pad(0.1));
                }
            } catch (e) { console.error("Error:", e); }
        }

        function renderMarkers(type) {
            markerLayer.clearLayers();
            rawData.forEach(p => {
                if (type === 'ทั้งหมด' || p.type === type) {
                    L.marker([p.lat, p.lng]).addTo(markerLayer)
                     .bindPopup(`<b>จุดทิ้งขยะ: ${p.name}</b><br>ประเภท: ${p.type}`);
                }
            });
        }

        function filterMap(type, btn) {
            document.querySelectorAll('.btn-outline').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            renderMarkers(type);
        }

        function findMyLocation() {
            if (navigator.geolocation) {
                navigator.geolocation.getCurrentPosition(function(pos) {
                    const lat = pos.coords.latitude;
                    const lng = pos.coords.longitude;
                    if (userMarker) { map.removeLayer(userMarker); }
                    userMarker = L.marker([lat, lng]).addTo(map).bindPopup("<b>คุณอยู่ที่นี่</b>").openPopup();
                    map.setView([lat, lng], 17);
                }, () => alert("กรุณาเปิด GPS"));
            }
        }

        async function sendReport() {
            const btn = document.getElementById('reportBtn');
            navigator.geolocation.getCurrentPosition(async (pos) => {
                btn.disabled = true; btn.innerText = "กำลังบันทึกข้อมูล...";
                const data = { lat: pos.coords.latitude, lng: pos.coords.longitude, issue: document.getElementById('issueType').value };
                try {
                    await fetch(SCRIPT_URL, { method: 'POST', body: JSON.stringify(data), mode: 'no-cors' });
                    alert("บันทึกข้อมูลเรียบร้อย ขอบคุณที่ช่วยดูแลเมืองครับ!");
                } catch (e) { alert("ล้มเหลว: " + e); }
                btn.disabled = false; btn.innerText = "🚀 ส่ง
