const express = require('express');
const cors    = require('cors');
const path    = require('path');
const fs      = require('fs');

const compressRoutes = require('./routes/compress');

const app  = express();
const PORT = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

// Creer les dossiers necessaires au demarrage
const UPLOAD_DIR  = path.join(__dirname, 'uploads');
const RESULTS_DIR = path.join(__dirname, 'results');

if (!fs.existsSync(UPLOAD_DIR))  fs.mkdirSync(UPLOAD_DIR,  { recursive: true });
if (!fs.existsSync(RESULTS_DIR)) fs.mkdirSync(RESULTS_DIR, { recursive: true });

// Routes
app.use('/api', compressRoutes);

// Health check
app.get('/api/health', (req, res) => {
    res.json({
        status: 'ok',
        message: 'AlgoZip API is running',
        version: '1.0.0',
        binary: process.env.ALGOZIP_BINARY || '../build/algozip'
    });
});

// Servir index.html pour toutes les autres routes
app.get('*', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

app.listen(PORT, () => {
    console.log(`AlgoZip Web Interface running on port ${PORT}`);
});
