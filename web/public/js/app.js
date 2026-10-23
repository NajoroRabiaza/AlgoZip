// State
let currentAction = 'compress';
let currentMode   = 'bzip2';
let selectedFile  = null;
let isRunning     = false;

// Theme
function toggleTheme() {
    const html   = document.documentElement;
    const sw     = document.getElementById('themeSwitch');
    const isDark = html.getAttribute('data-theme') === 'dark';
    html.setAttribute('data-theme', isDark ? 'light' : 'dark');
    sw.classList.toggle('active', !isDark);
    document.querySelector('.theme-toggle span').textContent =
        isDark ? 'Light' : 'Dark';
}

// Sidebar selection
function selectAction(action, mode, el) {
    document.querySelectorAll('.sidebar-item')
            .forEach(i => i.classList.remove('active'));
    el.classList.add('active');

    currentAction = action;
    if (mode) currentMode = mode;

    const isInfo = action === 'health' || action === 'stats';

    document.getElementById('uploadGroup').style.display =
        isInfo ? 'none' : 'flex';
    document.getElementById('modeGroup').style.display  =
        isInfo ? 'none' : 'flex';
    document.getElementById('infoGroup').style.display  =
        isInfo ? 'flex' : 'none';

    document.getElementById('panelTitle').textContent =
        isInfo ? 'API Info' : `${action.charAt(0).toUpperCase() + action.slice(1)} file`;

    document.getElementById('hintText').textContent = isInfo
        ? 'Click Run to fetch API information.'
        : `Upload a file and click Run to ${action} it with ${mode}.`;

    if (mode) {
        document.getElementById('btn-bzip2').classList.toggle('active', mode === 'bzip2');
        document.getElementById('btn-lzma').classList.toggle('active',  mode === 'lzma');
    }
}

// Mode selection
function selectMode(mode) {
    currentMode = mode;
    document.getElementById('btn-bzip2').classList.toggle('active', mode === 'bzip2');
    document.getElementById('btn-lzma').classList.toggle('active',  mode === 'lzma');
}

// File handling
function handleFileSelect(event) {
    const file = event.target.files[0];
    if (file) setFile(file);
}

function handleDragOver(event) {
    event.preventDefault();
    document.getElementById('uploadArea').classList.add('dragover');
}

function handleDrop(event) {
    event.preventDefault();
    document.getElementById('uploadArea').classList.remove('dragover');
    const file = event.dataTransfer.files[0];
    if (file) setFile(file);
}

function setFile(file) {
    selectedFile = file;
    document.getElementById('fileName').textContent = file.name;
    document.getElementById('fileSize').textContent = formatBytes(file.size);
    document.getElementById('fileInfo').classList.add('visible');
}

function formatBytes(bytes) {
    if (bytes < 1024)        return bytes + ' B';
    if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
    return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
}

// Console
function clearConsole() {
    const body = document.getElementById('consoleBody');
    body.innerHTML = `
        <div class="console-empty" id="consoleEmpty">
            <div class="console-cursor">&#x276F;_</div>
            <div class="console-hint">Hit Run to see the response here.</div>
        </div>`;
}

function appendLog(text, cssClass) {
    const empty = document.getElementById('consoleEmpty');
    if (empty) empty.remove();

    const body = document.getElementById('consoleBody');
    const line = document.createElement('div');
    line.className = `log-line ${cssClass || 'log-normal'}`;
    line.textContent = '> ' + text;
    body.appendChild(line);
    body.scrollTop = body.scrollHeight;
}

function appendResult(data) {
    const empty = document.getElementById('consoleEmpty');
    if (empty) empty.remove();

    const body = document.getElementById('consoleBody');
    const card = document.createElement('div');
    card.className = 'result-card';
    card.innerHTML = `
        <div class="result-title">Compression Complete</div>
        <div class="result-row">
            <span class="result-key">Original size</span>
            <span class="result-value">${formatBytes(data.inputSize)}</span>
        </div>
        <div class="result-row">
            <span class="result-key">Compressed size</span>
            <span class="result-value">${formatBytes(data.outputSize)}</span>
        </div>
        <div class="result-row">
            <span class="result-key">Ratio</span>
            <span class="result-value">${data.ratio}%</span>
        </div>
        <div class="result-row">
            <span class="result-key">Algorithm</span>
            <span class="result-value">${currentMode.toUpperCase()}</span>
        </div>
        <button class="download-btn"
                onclick="downloadResult('${data.downloadId}', '${data.filename}')">
            Download result
        </button>`;
    body.appendChild(card);
    body.scrollTop = body.scrollHeight;
}

// Download
function downloadResult(id, originalName) {
    const a = document.createElement('a');
    a.href = `/api/download/${id}`;
    a.download = originalName + '.az';
    a.click();
}

// Run
async function run() {
    if (isRunning) return;

    const isInfo = currentAction === 'health' || currentAction === 'stats';

    if (!isInfo && !selectedFile) {
        appendLog('Please select a file first.', 'log-error');
        return;
    }

    isRunning = true;
    document.getElementById('runBtn').disabled = true;
    clearConsole();

    if (isInfo) {
        await runInfo();
    } else {
        await runCompress();
    }

    isRunning = false;
    document.getElementById('runBtn').disabled = false;
}

async function runInfo() {
    const endpoint = currentAction === 'health' ? '/api/health' : '/api/stats';
    appendLog(`GET ${endpoint}`, 'log-info');

    try {
        const res  = await fetch(endpoint);
        const data = await res.json();
        appendLog(JSON.stringify(data, null, 2), 'log-success');
    } catch (err) {
        appendLog('Error: ' + err.message, 'log-error');
    }
}

async function runCompress() {
    const formData = new FormData();
    formData.append('file',   selectedFile);
    formData.append('mode',   currentMode);
    formData.append('action', currentAction);

    appendLog(`${currentAction.toUpperCase()} with ${currentMode}`, 'log-start');
    appendLog(`File: ${selectedFile.name} (${formatBytes(selectedFile.size)})`, 'log-info');

    try {
        const response = await fetch('/api/compress', {
            method: 'POST',
            body:   formData
        });

        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }

        const reader  = response.body.getReader();
        const decoder = new TextDecoder();
        let   buffer  = '';

        while (true) {
            const { done, value } = await reader.read();
            if (done) break;

            buffer += decoder.decode(value, { stream: true });
            const lines = buffer.split('\n');
            buffer = lines.pop();

            for (const line of lines) {
                if (!line.startsWith('data:')) continue;
                try {
                    const data = JSON.parse(line.slice(5).trim());
                    handleSSEData(data);
                } catch {}
            }
        }

    } catch (err) {
        appendLog('Error: ' + err.message, 'log-error');
    }
}

function handleSSEData(data) {
    switch (data.type) {
        case 'start':
            appendLog(data.message, 'log-start');
            break;
        case 'log':
            // Detecter les lignes de blocs pour les colorer differemment
            if (data.message.startsWith('Block')) {
                appendLog(data.message, 'log-block');
            } else {
                appendLog(data.message, 'log-normal');
            }
            break;
        case 'error':
            appendLog(data.message, 'log-error');
            break;
        case 'success':
            appendLog(data.message, 'log-success');
            appendResult(data);
            break;
    }
}
