// This express router handles serving all web content to clients.
// This includes HTML, CSS, JS, and other static resources.

// Router class from the routing framework.
import express from 'express'
// Library for filepath manipulation.
import path from 'path';
// Middleware for compiling less files on-the-fly.
import less from 'less-middleware';

// Create a router for serving web resources.
const router = express.Router();

// Set up generation of css from less.
router.use('/res/styles', less(path.join(__dirname, '../../less'), {
  debug: process.env.NODE_ENV === 'development',
  dest: path.join(__dirname, '../../res/styles'),
  force: process.env.NODE_ENV == 'development',
}));

// Set up routes for static content under ../../res.
router.use('/res', express.static(path.join(__dirname, '../../res'), {
  index: false,  // Don't generate index files for directories.
}));

// Routes for dynamically rendered content:

// Landing page.
router.get('/', function(req, res, next) {
  res.render('conversations');
});

// Treat all unhandled requests under /web as a 404.
router.use(function (req, res, next) {
  let err = new Error('Not Found');
  err.status = 404;
  return next(err);
});

// Return a web rendered error for all unhandled errors in this router.
router.use(function (err, req, res, next) {
  return res.status(err.status || 500).send(`<p> Error ${err.status || 500} </p><p> ${err.message} </p>`);
});

// Export the router from this module.
export default router;
