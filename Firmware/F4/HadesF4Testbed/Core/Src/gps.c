#include "gps.h"


#define FLT(x)              ((gps_float_t)(x))
#define D2R(x)              FLT(FLT(x) * FLT(0.01745329251994)) /*!< Degrees to radians */
#define R2D(x)              FLT(FLT(x) * FLT(57.29577951308232))/*!< Radians to degrees */
#define EARTH_RADIUS        FLT(6371.0) /*!< Earth radius in units of kilometers */

#define STAT_UNKNOWN        0
#define STAT_GGA            1
#define STAT_GSA            2
#define STAT_GSV            3
#define STAT_RMC            4

#define CRC_ADD(_gh, ch)    (_gh)->p.crc_calc ^= (uint8_t)(ch)
#define TERM_ADD(_gh, ch)   do {    \
    if ((_gh)->p.term_pos < (sizeof((_gh)->p.term_str) - 1)) {  \
        (_gh)->p.term_str[(_gh)->p.term_pos++] = (ch);  \
        (_gh)->p.term_str[(_gh)->p.term_pos] = 0;   \
    }                               \
} while (0)
#define TERM_NEXT(_gh)      do { (_gh)->p.term_str[((_gh)->p.term_pos = 0)] = 0; (_gh)->p.term_num++; } while (0)

#define CIN(x)              ((x) >= '0' && (x) <= '9')
#define CIHN(x)             (((x) >= '0' && (x) <= '9') || ((x) >= 'a' && (x) <= 'f') || ((x) >= 'A' && (x) <= 'F'))
#define CTN(x)              ((x) - '0')
#define CHTN(x)             (((x) >= '0' && (x) <= '9') ? ((x) - '0') : (((x) >= 'a' && (x) <= 'z') ? ((x) - 'a' + 10) : (((x) >= 'A' && (x) <= 'Z') ? ((x) - 'A' + 10) : 0)))

/**
 * \brief           Parse number as integer
 * \param[in]       gh: GPS handle
 * \param[in]       t: Text to parse. Set to `NULL` to parse current GPS term
 * \return          Parsed integer
 */
static int32_t
parse_number(gps_t* gh, const char* t) {
    int32_t res = 0;
    uint8_t minus;

    if (t == NULL) {
        t = gh->p.term_str;
    }
    for (; t != NULL && *t == ' '; t++) {}      /* Strip leading spaces */

    minus = (*t == '-' ? (t++, 1) : 0);
    for (; t != NULL && CIN(*t); t++) {
        res = 10 * res + CTN(*t);
    }
    return minus ? -res : res;
}

/**
 * \brief           Parse number as double and convert it to \ref gps_float_t
 * \param[in]       gh: GPS handle
 * \param[in]       t: Text to parse. Set to `NULL` to parse current GPS term
 * \return          Parsed double in \ref gps_float_t format
 */
static gps_float_t
parse_float_number(gps_t* gh, const char* t) {
    gps_float_t res;

    if (t == NULL) {
        t = gh->p.term_str;
    }
    for (; t != NULL && *t == ' '; t++) {}      /* Strip leading spaces */

    res = strtof(t, NULL);                      /* Parse string to float */


    return FLT(res);                            /* Return casted value, based on float size */
}

/**
 * \brief           Parse latitude/longitude NMEA format to double
 *
 *                  NMEA output for latitude is ddmm.sss and longitude is dddmm.sss
 * \param[in]       gh: GPS handle
 * \return          Latitude/Longitude value in degrees
 */
static gps_float_t
parse_lat_long(gps_t* gh) {
    gps_float_t ll, deg, min;

    ll = parse_float_number(gh, NULL);          /* Parse value as double */
    deg = FLT((int)((int)ll / 100));            /* Get absolute degrees value, interested in integer part only */
    min = ll - (deg * FLT(100));                /* Get remaining part from full number, minutes */
    ll = deg + (min / FLT(60.0));               /* Calculate latitude/longitude */

    return ll;
}

/**
 * \brief           Parse received term
 * \param[in]       gh: GPS handle
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
parse_term(gps_t* gh) {
    if (gh->p.term_num == 0) {                  /* Check string type */
        if (0) {
#if GPS_CFG_STATEMENT_GPGGA
        } else if (!strncmp(gh->p.term_str, "$GPGGA", 6) || !strncmp(gh->p.term_str, "$GNGGA", 6)) {
            gh->p.stat = STAT_GGA;
#endif /* GPS_CFG_STATEMENT_GPGGA */
#if GPS_CFG_STATEMENT_GPGSA
        } else if (!strncmp(gh->p.term_str, "$GPGSA", 6) || !strncmp(gh->p.term_str, "$GNGSA", 6)) {
            gh->p.stat = STAT_GSA;
#endif /* GPS_CFG_STATEMENT_GPGSA */
#if GPS_CFG_STATEMENT_GPGSV
        } else if (!strncmp(gh->p.term_str, "$GPGSV", 6) || !strncmp(gh->p.term_str, "$GNGSV", 6)) {
            gh->p.stat = STAT_GSV;
#endif /* GPS_CFG_STATEMENT_GPGSV */
#if GPS_CFG_STATEMENT_GPRMC
        } else if (!strncmp(gh->p.term_str, "$GPRMC", 6) || !strncmp(gh->p.term_str, "$GNRMC", 6)) {
            gh->p.stat = STAT_RMC;
#endif /* GPS_CFG_STATEMENT_GPRMC */
        } else {
            gh->p.stat = STAT_UNKNOWN;          /* Invalid statement for library */
        }
        return 1;
    }

    /* Start parsing terms */
    if (gh->p.stat == STAT_UNKNOWN) {
#if GPS_CFG_STATEMENT_GPGGA
    } else if (gh->p.stat == STAT_GGA) {        /* Process GPGGA statement */
        switch (gh->p.term_num) {
            case 1:                             /* Process UTC time */
                gh->p.data.gga.hours = 10 * CTN(gh->p.term_str[0]) + CTN(gh->p.term_str[1]);
                gh->p.data.gga.minutes = 10 * CTN(gh->p.term_str[2]) + CTN(gh->p.term_str[3]);
                gh->p.data.gga.seconds = 10 * CTN(gh->p.term_str[4]) + CTN(gh->p.term_str[5]);
                break;
            case 2:                             /* Latitude */
                gh->p.data.gga.latitude = parse_lat_long(gh);   /* Parse latitude */
                break;
            case 3:                             /* Latitude north/south information */
                if (gh->p.term_str[0] == 'S' || gh->p.term_str[0] == 's') {
                    gh->p.data.gga.latitude = -gh->p.data.gga.latitude;
                }
                break;
            case 4:                             /* Longitude */
                gh->p.data.gga.longitude = parse_lat_long(gh);  /* Parse longitude */
                break;
            case 5:                             /* Longitude east/west information */
                if (gh->p.term_str[0] == 'W' || gh->p.term_str[0] == 'w') {
                    gh->p.data.gga.longitude = -gh->p.data.gga.longitude;
                }
                break;
            case 6:                             /* Fix status */
                gh->p.data.gga.fix = (uint8_t)parse_number(gh, NULL);
                break;
            case 7:                             /* Satellites in use */
                gh->p.data.gga.sats_in_use = (uint8_t)parse_number(gh, NULL);
                break;
            case 9:                             /* Altitude */
                gh->p.data.gga.altitude = parse_float_number(gh, NULL);
                break;
            case 11:                            /* Altitude above ellipsoid */
                gh->p.data.gga.geo_sep = parse_float_number(gh, NULL);
                break;
            default: break;
        }
#endif /* GPS_CFG_STATEMENT_GPGGA */
#if GPS_CFG_STATEMENT_GPGSA
    } else if (gh->p.stat == STAT_GSA) {        /* Process GPGSA statement */
        switch (gh->p.term_num) {
            case 2:                             /* Process fix mode */
                gh->p.data.gsa.fix_mode = (uint8_t)parse_number(gh, NULL);
                break;
            case 15:                            /* Process PDOP */
                gh->p.data.gsa.dop_p = parse_float_number(gh, NULL);
                break;
            case 16:                            /* Process HDOP */
                gh->p.data.gsa.dop_h = parse_float_number(gh, NULL);
                break;
            case 17:                            /* Process VDOP */
                gh->p.data.gsa.dop_v = parse_float_number(gh, NULL);
                break;
            default:
                /* Parse satellite IDs */
                if (gh->p.term_num >= 3 && gh->p.term_num <= 14) {
                    gh->p.data.gsa.satellites_ids[gh->p.term_num - 3] = (uint8_t)parse_number(gh, NULL);
                }
                break;
        }
#endif /* GPS_CFG_STATEMENT_GPGSA */
#if GPS_CFG_STATEMENT_GPGSV
    } else if (gh->p.stat == STAT_GSV) {        /* Process GPGSV statement */
        switch (gh->p.term_num) {
            case 2:                             /* Current GPGSV statement number */
                gh->p.data.gsv.stat_num = (uint8_t)parse_number(gh, NULL);
                break;
            case 3:                             /* Process satellites in view */
                gh->p.data.gsv.sats_in_view = (uint8_t)parse_number(gh, NULL);
                break;
            default:
#if GPS_CFG_STATEMENT_GPGSV_SAT_DET
                if (gh->p.term_num >= 4 && gh->p.term_num <= 19) {  /* Check current term number */
                    uint8_t index, term_num = gh->p.term_num - 4;   /* Normalize term number from 4-19 to 0-15 */
                    uint16_t value;

                    index = 4 * (gh->p.data.gsv.stat_num - 1) + term_num / 4;   /* Get array index */
                    if (index < sizeof(gh->sats_in_view_desc) / sizeof(gh->sats_in_view_desc[0])) {
                        value = (uint16_t)parse_number(gh, NULL);   /* Parse number as integer */
                        switch (term_num % 4) {
                            case 0: gh->sats_in_view_desc[index].num = value; break;
                            case 1: gh->sats_in_view_desc[index].elevation = value; break;
                            case 2: gh->sats_in_view_desc[index].azimuth = value; break;
                            case 3: gh->sats_in_view_desc[index].snr = value; break;
                            default: break;
                        }
                    }
                }
#endif /* GPS_CFG_STATEMENT_GPGSV_SAT_DET */
                break;
        }
#endif /* GPS_CFG_STATEMENT_GPGSV */
#if GPS_CFG_STATEMENT_GPRMC
    } else if (gh->p.stat == STAT_RMC) {        /* Process GPRMC statement */
        switch (gh->p.term_num) {
            case 2:                             /* Process valid status */
                gh->p.data.rmc.is_valid = (gh->p.term_str[0] == 'A');
                break;
            case 7:                             /* Process ground speed in knots */
                gh->p.data.rmc.speed = parse_float_number(gh, NULL);
                break;
            case 8:                             /* Process true ground coarse */
                gh->p.data.rmc.coarse = parse_float_number(gh, NULL);
                break;
            case 9:                             /* Process date */
                gh->p.data.rmc.date = (uint8_t)(10 * CTN(gh->p.term_str[0]) + CTN(gh->p.term_str[1]));
                gh->p.data.rmc.month = (uint8_t)(10 * CTN(gh->p.term_str[2]) + CTN(gh->p.term_str[3]));
                gh->p.data.rmc.year = (uint8_t)(10 * CTN(gh->p.term_str[4]) + CTN(gh->p.term_str[5]));
                break;
            case 10:                            /* Process magnetic variation */
                gh->p.data.rmc.variation = parse_float_number(gh, NULL);
                break;
            case 11:                            /* Process magnetic variation east/west */
                if (gh->p.term_str[0] == 'W' || gh->p.term_str[0] == 'w') {
                    gh->p.data.rmc.variation = -gh->p.data.rmc.variation;
                }
                break;
            default: break;
        }
#endif /* GPS_CFG_STATEMENT_GPRMC */
    }
    return 1;
}

/**
 * \brief           Compare calculated CRC with received CRC
 * \param[in]       gh: GPS handle
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
check_crc(gps_t* gh) {
    uint8_t crc;
    crc = (uint8_t)((CHTN(gh->p.term_str[0]) & 0x0F) << 0x04) | (CHTN(gh->p.term_str[1]) & 0x0F);   /* Convert received CRC from string (hex) to number */
    return gh->p.crc_calc == crc;               /* They must match! */
}

/**
 * \brief           Copy temporary memory to user memory
 * \param[in]       gh: GPS handle
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
copy_from_tmp_memory(gps_t* gh) {
    if (0) {
#if GPS_CFG_STATEMENT_GPGGA
    } else if (gh->p.stat == STAT_GGA) {
        gh->latitude = gh->p.data.gga.latitude;
        gh->longitude = gh->p.data.gga.longitude;
        gh->altitude = gh->p.data.gga.altitude;
        gh->geo_sep = gh->p.data.gga.geo_sep;
        gh->sats_in_use = gh->p.data.gga.sats_in_use;
        gh->fix = gh->p.data.gga.fix;
        gh->hours = gh->p.data.gga.hours;
        gh->minutes = gh->p.data.gga.minutes;
        gh->seconds = gh->p.data.gga.seconds;
#endif /* GPS_CFG_STATEMENT_GPGGA */
#if GPS_CFG_STATEMENT_GPGSA
    } else if (gh->p.stat == STAT_GSA) {
        gh->dop_h = gh->p.data.gsa.dop_h;
        gh->dop_p = gh->p.data.gsa.dop_p;
        gh->dop_v = gh->p.data.gsa.dop_v;
        gh->fix_mode = gh->p.data.gsa.fix_mode;
        memcpy(gh->satellites_ids, gh->p.data.gsa.satellites_ids, sizeof(gh->satellites_ids));
#endif /* GPS_CFG_STATEMENT_GPGSA */
#if GPS_CFG_STATEMENT_GPGSV
    } else if (gh->p.stat == STAT_GSV) {
        gh->sats_in_view = gh->p.data.gsv.sats_in_view;
#endif /* GPS_CFG_STATEMENT_GPGSV */
#if GPS_CFG_STATEMENT_GPRMC
    } else if (gh->p.stat == STAT_RMC) {
        gh->coarse = gh->p.data.rmc.coarse;
        gh->is_valid = gh->p.data.rmc.is_valid;
        gh->speed = gh->p.data.rmc.speed;
        gh->variation = gh->p.data.rmc.variation;
        gh->date = gh->p.data.rmc.date;
        gh->month = gh->p.data.rmc.month;
        gh->year = gh->p.data.rmc.year;
#endif /* GPS_CFG_STATEMENT_GPRMC */
    }
    return 1;
}

/**
 * \brief           Init GPS handle
 * \param[in]       gh: GPS handle structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
gps_init(gps_t* gh) {
    memset(gh, 0x00, sizeof(*gh));              /* Reset structure */
    return 1;
}

/**
 * \brief           Process NMEA data from GPS receiver
 * \param[in]       gh: GPS handle structure
 * \param[in]       data: Received data
 * \param[in]       len: Number of bytes to process
 * \return          `1` on success, `0` otherwise
 */
uint8_t
gps_process(gps_t* gh, const void* data, size_t len) {
    const uint8_t* d = data;

    while (len--) {                             /* Process all bytes */
        if (*d == '$') {                        /* Check for beginning of NMEA line */
            memset(&gh->p, 0x00, sizeof(gh->p));/* Reset private memory */
            TERM_ADD(gh, *d);                   /* Add character to term */
        } else if (*d == ',') {                 /* Term separator character */
            parse_term(gh);                     /* Parse term we have currently in memory */
            CRC_ADD(gh, *d);                    /* Add character to CRC computation */
            TERM_NEXT(gh);                      /* Start with next term */
        } else if (*d == '*') {                 /* Start indicates end of data for CRC computation */
            parse_term(gh);                     /* Parse term we have currently in memory */
            gh->p.star = 1;                     /* STAR detected */
            TERM_NEXT(gh);                      /* Start with next term */
        } else if (*d == '\r') {
            if (check_crc(gh)) {                /* Check for CRC result */
                /* CRC is OK, in theory we can copy data from statements to user data */
                copy_from_tmp_memory(gh);       /* Copy memory from temporary to user memory */
            }
        } else {
            if (!gh->p.star) {                  /* Add to CRC only if star not yet detected */
                CRC_ADD(gh, *d);                /* Add to CRC */
            }
            TERM_ADD(gh, *d);                   /* Add character to term */
        }
        d++;                                    /* Process next character */
    }
    return 1;
}

/**
 * \brief           Calculate distance and bearing between `2` latitude and longitude coordinates
 * \param[in]       las: Latitude start coordinate, in units of degrees
 * \param[in]       los: Longitude start coordinate, in units of degrees
 * \param[in]       lae: Latitude end coordinate, in units of degrees
 * \param[in]       loe: Longitude end coordinate, in units of degrees
 * \param[out]      d: Pointer to output distance in units of meters
 * \param[out]      b: Pointer to output bearing between start and end coordinate in relation to north in units of degrees
 * \return          `1` on success, `0` otherwise
 */
uint8_t
gps_distance_bearing(gps_float_t las, gps_float_t los, gps_float_t lae, gps_float_t loe, gps_float_t* d, gps_float_t* b) {
    gps_float_t df, dfi, a;

    if (d == NULL && b == NULL) {
        return 0;
    }

    /* Convert degrees to radians */
    df = D2R(lae - las);
    dfi = D2R(loe - los);
    las = D2R(las);
    los = D2R(los);
    lae = D2R(lae);
    loe = D2R(loe);

    /*
     * Calculate distance
     *
     * Calculated distance is absolute value in meters between 2 points on earth.
     */
    if (d != NULL) {
        /*
         * a = sin(df / 2)^2 + cos(las) * cos(lae) * sin(dfi / 2)^2
         * *d = RADIUS * 2 * atan(a / (1 - a)) * 1000 (for meters)
         */
#if GPS_CFG_DOUBLE
        a = FLT(sin(df * 0.5) * sin(df * 0.5) + sin(dfi * 0.5) * sin(dfi * 0.5) * cos(las) * cos(lae));
        *d = FLT(EARTH_RADIUS * 2.0 * atan2(sqrt(a), sqrt(1.0 - a)) * 1000.0);
#else /* GPS_CFG_DOUBLE */
        a = FLT(sinf(df * 0.5f) * sinf(df * 0.5f) + sinf(dfi * 0.5f) * sinf(dfi * 0.5f) * cosf(las) * cosf(lae));
        *d = FLT(EARTH_RADIUS * 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a)) * 1000.0f);
#endif /* !GPS_CFG_DOUBLE */
    }

    /*
     * Calculate bearing
     *
     * Bearing is calculated from point 1 to point 2.
     * Result will tell us in which direction (according to north) we should move,
     * to reach point 2.
     *
     * Example:
     *      Bearing is 0 => move to north
     *      Bearing is 90 => move to east
     *      Bearing is 180 => move to south
     *      Bearing is 270 => move to west
     */
    if (b != NULL) {
#if GPS_CFG_DOUBLE
        df = FLT(sin(loe - los) * cos(lae));
        dfi = FLT(cos(las) * sin(lae) - sin(las) * cos(lae) * cos(loe - los));

        *b = R2D(atan2(df, dfi));               /* Calculate bearing and convert to degrees */
#else /* GPS_CFG_DOUBLE */
        df = FLT(sinf(loe - los) * cosf(lae));
        dfi = FLT(cosf(las) * sinf(lae) - sinf(las) * cosf(lae) * cosf(loe - los));

        *b = R2D(atan2f(df, dfi));              /* Calculate bearing and convert to degrees */
#endif /* !GPS_CFG_DOUBLE */
        if (*b < 0) {                           /* Check for negative angle */
            *b += FLT(360);                     /* Make bearing always positive */
        }
    }
    return 1;
}

/**
 * \brief           Convert NMEA GPS speed (in knots = nautical mile per hour) to different speed format
 * \param[in]       sik: Speed in knots, received from GPS NMEA statement
 * \param[in]       ts: Target speed to convert to from knots
 * \return          Speed calculated from knots
 */
gps_float_t
gps_to_speed(gps_float_t sik, gps_speed_t ts) {
    switch (ts) {
        case gps_speed_kps:     return FLT(sik * FLT(0.000514));
        case gps_speed_kph:     return FLT(sik * FLT(1.852));
        case gps_speed_mps:     return FLT(sik * FLT(0.5144));
        case gps_speed_mpm:     return FLT(sik * FLT(30.87));

        case gps_speed_mips:    return FLT(sik * FLT(0.0003197));
        case gps_speed_mph:     return FLT(sik * FLT(1.151));
        case gps_speed_fps:     return FLT(sik * FLT(1.688));
        case gps_speed_fpm:     return FLT(sik * FLT(101.3));

        case gps_speed_mpk:     return FLT(sik * FLT(32.4));
        case gps_speed_spk:     return FLT(sik * FLT(1944.0));
        case gps_speed_sp100m:  return FLT(sik * FLT(194.4));
        case gps_speed_mipm:    return FLT(sik * FLT(52.14));
        case gps_speed_spm:     return FLT(sik * FLT(3128.0));
        case gps_speed_sp100y:  return FLT(sik * FLT(177.7));

        case gps_speed_smph:    return FLT(sik * FLT(1.0));
        default: return 0;
    }
}
