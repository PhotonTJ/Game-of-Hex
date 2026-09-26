import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// Dev proxy so the frontend can call relative '/api' and '/ws' paths in both
// `npm run dev` and the production nginx build (see frontend/nginx.conf).
export default defineConfig({
  plugins: [react()],
  server: {
    proxy: {
      '/api': { target: 'http://localhost:8000', changeOrigin: true },
      '/ws': { target: 'ws://localhost:8000', ws: true },
    },
  },
})
