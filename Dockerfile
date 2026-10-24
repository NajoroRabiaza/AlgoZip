FROM node:20-slim

# Installer les dependances systeme
RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Copier le code source
WORKDIR /app
COPY . .

# Compiler AlgoZip
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make algozip -j$(nproc)

# Installer les dependances Node.js
RUN npm install -g pnpm && \
    cd web && pnpm install

# Variables d environnement
ENV PORT=3000
ENV ALGOZIP_BINARY=/app/build/algozip
ENV NODE_ENV=production

# Exposer le port
EXPOSE 3000

# Lancer le serveur
CMD ["node", "web/server.js"]
