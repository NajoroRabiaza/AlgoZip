const express  = require('express');
const multer   = require('multer');
const { v4: uuidv4 } = require('uuid');
const path     = require('path');
const fs       = require('fs');
const { spawn } = require('child_process');

const router = express.Router();

const UPLOAD_DIR  = path.join(__dirname, '..', 'uploads');
const RESULTS_DIR = path.join(__dirname, '..', 'results');

// Chemin vers le binaire algozip
// Sur Render : le binaire est compile dans ../build/algozip
const ALGOZIP = process.env.ALGOZIP_BINARY ||
                path.join(__dirname, '..', '..', 'build', 'algozip');

// Configuration multer pour l'upload
const storage = multer.diskStorage({
    destination: (req, file, cb) => cb(null, UPLOAD_DIR),
    filename:    (req, file, cb) => {
        const ext = path.extname(file.originalname);
        cb(null, uuidv4() + ext);
    }
});

const upload = multer({
    storage,
    limits: { fileSize: 50 * 1024 * 1024 } // 50MB max
});

// POST /api/compress
// Upload + compression avec streaming des logs via SSE
router.post('/compress', upload.single('file'), (req, res) => {
    if (!req.file) {
        return res.status(400).json({ error: 'No file uploaded' });
    }

    const mode       = req.body.mode || 'bzip2';
    const action     = req.body.action || 'compress';
    const inputPath  = req.file.path;
    const outputName = uuidv4() + '.az';
    const outputPath = path.join(RESULTS_DIR, outputName);

    // Headers pour Server-Sent Events
    // Ca permet de streamer les logs en temps reel vers le frontend
    res.setHeader('Content-Type',  'text/event-stream');
    res.setHeader('Cache-Control', 'no-cache');
    res.setHeader('Connection',    'keep-alive');
    res.setHeader('Access-Control-Allow-Origin', '*');

    const sendLog = (data) => {
        res.write(`data: ${JSON.stringify(data)}\n\n`);
    };

    sendLog({ type: 'start', message: `Starting ${action} with ${mode}...` });
    sendLog({ type: 'log',   message: `Input file: ${req.file.originalname}` });
    sendLog({ type: 'log',   message: `File size: ${req.file.size} bytes` });

    // Lancer le binaire algozip
    const args = [action, '--mode', mode, inputPath, outputPath];
    const proc = spawn(ALGOZIP, args);

    // Streamer stdout en temps reel
    proc.stdout.on('data', (data) => {
        const lines = data.toString().split('\n').filter(l => l.trim());
        lines.forEach(line => {
            sendLog({ type: 'log', message: line });
        });
    });

    // Streamer stderr
    proc.stderr.on('data', (data) => {
        sendLog({ type: 'error', message: data.toString() });
    });

    // Fin du process
    proc.on('close', (code) => {
        if (code === 0 && fs.existsSync(outputPath)) {
            const outputSize = fs.statSync(outputPath).size;
            const inputSize  = req.file.size;
            const ratio      = ((outputSize / inputSize) * 100).toFixed(2);

            sendLog({
                type:       'success',
                message:    'Operation completed successfully',
                inputSize,
                outputSize,
                ratio,
                downloadId: outputName,
                filename:   req.file.originalname
            });
        } else {
            sendLog({
                type:    'error',
                message: `Process exited with code ${code}`
            });
        }

        // Nettoyer le fichier uploaded apres traitement
        fs.unlink(inputPath, () => {});

        res.end();
    });

    proc.on('error', (err) => {
        sendLog({ type: 'error', message: `Failed to start process: ${err.message}` });
        res.end();
    });
});

// GET /api/download/:id
// Telecharger le fichier resultat
router.get('/download/:id', (req, res) => {
    const filePath = path.join(RESULTS_DIR, req.params.id);

    if (!fs.existsSync(filePath)) {
        return res.status(404).json({ error: 'File not found' });
    }

    res.download(filePath, req.params.id, (err) => {
        if (!err) {
            // Supprimer le fichier apres telechargement
            fs.unlink(filePath, () => {});
        }
    });
});

// GET /api/stats
router.get('/stats', (req, res) => {
    res.json({
        algorithms: ['bzip2', 'lzma'],
        pipelines: {
            bzip2: 'BWT + Huffman',
            lzma:  'LZ77 + Arithmetic Coding'
        },
        maxFileSize: '50MB',
        blockSize:   '1KB'
    });
});

module.exports = router;
