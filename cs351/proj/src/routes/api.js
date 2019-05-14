// This express router handles serving all JSON based api requests.

// Router class from the routing framework.
import express from 'express'
// Mysql library for databse interactions.
import mysql from 'mysql2'
// Files system and path utilities.
import path from 'path'
import fs from 'fs'

// Using require since this package doesn't have obvious exports.
// This just helps express handle async errors better.
require('express-async-errors');

// Create a router for serving web resources.
const router = express.Router();

// Mysql connection pool.
const pool = mysql.createPool({
  host: 'localhost',
  user: 'e2ee',
  database: 'e2ee',
  password: fs.readFileSync(path.join(__dirname, '../../.dbpass'), { encoding: 'utf-8' }).trim(),
}).promise();  // Let's us use the promise API for this pool.

router.use('/', async (req, res, next) => {
  res.locals.conn = await pool.getConnection();
  return next();
});

// Treat all unhandled requests under /api as a 404.
router.use((req, res, next) => {
  let err = new Error('Not Found');
  err.status = 404;
  return next(err);
});

// Return a JSON error for all unhandled errors in this router.
router.use((err, req, res, next) => {
  res.status(err.status || 500);
  res.json({ status: res.status, message: err.message });
  return next();
});

router.use('/', (req, res, next) => {
  if (res.locals.conn) {
    res.locals.conn.release();
  }
});

// Export the router from this module.
export default router;
