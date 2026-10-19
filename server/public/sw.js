const CACHE = 'swasthlink-v1';
const ASSETS = ['/worker.html', '/triage.js', '/manifest.json'];
self.addEventListener('install', e => e.waitUntil(caches.open(CACHE).then(c => c.addAll(ASSETS)).then(() => self.skipWaiting())));
self.addEventListener('activate', e => e.waitUntil(self.clients.claim()));
self.addEventListener('fetch', e => {
  const u = new URL(e.request.url);
  if (u.pathname.startsWith('/api/')) return;               // API: network only (the app queues writes itself)
  e.respondWith(caches.match(e.request).then(r => r || fetch(e.request)));
});
