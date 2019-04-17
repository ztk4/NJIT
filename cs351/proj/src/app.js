// This contains the high-level view of routing for the app.
// The app is segmented into two primary pieces:
//   1. Client Facing API:
//      - Creation and management of users and their metadata.
//      - Fascilitation of encrypted P2P messaging between pairs of users.
//   2. Web Serving:
//      - Serves client-side web-app content to each client.
// These segments are handled respectively under routes/{api.js,srv.js}.
//
// App-wide middleware is mixed-in to express at this level.

// Web routing and middleware framework.
import express from 'express';
// Server-side logging of web requests.
import logger from 'morgan';
// Cookie parsing logic (may or may not be useful).
import cookie_parser from 'cookie-parser';
// Body parsing logic (useful for extracting data from request body).
import body_parser from 'body-parser';
// Library for manipulating filepaths.
import path from 'path';

// Import our routers.
import web_router from './routes/web'

// Create an express app.
const app = express();

// Enable logging with verbosity appropriate for env.
app.use(logger(app.get('env') === 'development' ? 'dev' : 'combined'));
// Enables automatic body parsing of MIME application/json.
app.use(body_parser.json());
// Enables automatic parsing of cookies.
app.use(cookie_parser());
// Enables view rendering using the pug engine.
app.set('views', path.join(__dirname, '../views'));
app.set('view engine', 'pug');

// Attach router for api under /api.
// TODO: app.use('/api', api_router);
// Attach router for web serving under /web.
app.use('/web', web_router);

// Add a redirect from root (common landing page) to /web (desired landing page).
app.get('/', function (req, res, next) {
  return res.redirect('/web');
});

// Catch all unhandled requests and treat them as 404's.
app.use(function(req, res, next) {
  let err = new Error('Not Found');
  err.status = 404;
  return next(err);
});

// Export the express app from this module.
export default app;
