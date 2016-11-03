/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2012
   Author(s): Jonathan Poelen

   Unit test to detect memory leak in OCR module
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestNewOCR
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
// #define LOGPRINT

#include "capture/title_extractors/ocr_titles_extractor.hpp"
#include "capture/title_extractors/ocr_title_filter.hpp"

#include "utils/drawable.hpp"
#include "utils/bitmap_with_png.hpp"

namespace {
    template<class... Str>
    std::array<char const *, sizeof...(Str)>
    make_array(Str... str) {
        return {{str...}};
    }

    void draw_bitmap(Drawable & drawable, char const * bitmap_filename) {
        Bitmap bmp = bitmap_from_file(bitmap_filename);
        drawable.draw_bitmap({0, 0, drawable.width(), drawable.height()}, bmp);
    }
}

BOOST_AUTO_TEST_CASE(TestNewOCR1)
{
    Drawable drawable(800, 600);

    using ocr::locale::LocaleId;
    OcrTitlesExtractor extractor(true, 100, LocaleId{LocaleId::latin});

    OcrTitleFilter filter;
    std::vector<OcrTitle> out_titles;

    {
        draw_bitmap(drawable, "./tests/fixtures/m-21288-2.bmp");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "Gestionnaire de serveur"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/w7mediacentercapture.bmp");
        extractor.extract_titles(drawable, out_titles);

        BOOST_CHECK_EQUAL(out_titles.size(), 0);
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/win2008capture2.bmp");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "licences TSGestionnaire de licences TS",
            "Gestionnaire de licences TSGestionnaire de licences TS"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 1);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/win2008capture4.bmp");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "plop fi test fififid zjhufh .txt - Bloc-notes"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/win2008capture.bmp");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "Gestionnaire de licences TS"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/win2008capture3.bmp");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "Gestionnaire de serveur", "Gestionna? r de ? ?en? s TS"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/win2008capture5.bmp");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "ff fff ffff f fa fz fe fr ft fy fu fi fo fp fq fs fd fg fh fj fk fl fm fw fx fc fv fb fn f, f;"
            " f% fè fé f& f# fà f@ ffi p.txt - Bloc-notes"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    // extended ligatures ffff etc are unsupported yet
    {
        draw_bitmap(drawable, "./tests/fixtures/win2008capture6.bmp");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "ff fff ffff ? ? ? pp ff p fff p ffff p ? p ? p ? p.txt - Bloc-notes"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/win2008capture9.bmp");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "af rt rf th jk kk ok oo lo LM .txt - Bloc-notes"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/win2008capture10.bmp");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "qwhybcaliueLkaASsFkkUibnkzkwwkswq.txt - Bloc-notes"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }
}

BOOST_AUTO_TEST_CASE(Test2008R2)
{
    Drawable drawable(653, 244);

    using ocr::locale::LocaleId;
    OcrTitlesExtractor extractor(true, 100, LocaleId{LocaleId::latin});

    OcrTitleFilter filter;
    std::vector<OcrTitle> out_titles;

    {
        draw_bitmap(drawable, "./tests/fixtures/win2008R2_server_manager.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "Server Manager"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());
    }
}

BOOST_AUTO_TEST_CASE(TestNewOCR2)
{
    Drawable drawable(800, 600);

    using ocr::locale::LocaleId;
    OcrTitlesExtractor extractor(true, 100, LocaleId{LocaleId::latin});

    OcrTitleFilter filter;
    std::vector<OcrTitle> out_titles;

    {
        extractor.extract_titles(drawable, out_titles);
        BOOST_CHECK_EQUAL(out_titles.size(), 0);
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/win2008capture10.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "qwhybcaliueLkaASsFkkUibnkzkwwkswq.txt - Bloc-notes"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    BOOST_CHECK_EQUAL(
        extractor.selected_font_id(), ocr::fonts::font_id_by_name(LocaleId::latin, "tahoma_8_bold")
    );

    {
        draw_bitmap(drawable, "./tests/fixtures/"
                              "rdp@192.168.10.254,qaadministrateur@win2k3dc_rdp,20121128-114532,"
                              "wab2-3-1-0.yourdomain,8575-001616-000000.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            ".es"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }
}

BOOST_AUTO_TEST_CASE(TestNewOCR3)
{
    Drawable drawable(800, 600);

    using ocr::locale::LocaleId;
    OcrTitlesExtractor extractor(true, 100, LocaleId{LocaleId::latin});

    OcrTitleFilter filter;
    std::vector<OcrTitle> out_titles;

    {
        draw_bitmap(drawable, "./tests/fixtures/win2002capture1.png");
        extractor.set_font_id(ocr::fonts::font_id_by_name(ocr::fonts::LocaleId::latin, "tahoma_8_bold"));
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "toto"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }
}

BOOST_AUTO_TEST_CASE(TestNewOCR4)
{
    Drawable drawable(800, 600);

    using ocr::locale::LocaleId;
    OcrTitlesExtractor extractor(true, 100, LocaleId{LocaleId::latin});

    OcrTitleFilter filter;
    std::vector<OcrTitle> out_titles;

    {
        draw_bitmap(drawable, "./tests/fixtures/capture1.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "É.txt - Bloc-notes"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/capture2.png");
        extractor.set_font_id(-1);
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "Poste de travail"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }
}

BOOST_AUTO_TEST_CASE(TestNewOCRWin2012)
{
    Drawable drawable(1446, 927);

    using ocr::locale::LocaleId;
    OcrTitlesExtractor extractor(true, 100, LocaleId{LocaleId::latin});

    OcrTitleFilter filter;
    std::vector<OcrTitle> out_titles;

    {
        draw_bitmap(drawable, "./tests/fixtures/win2012capture2.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "Hyper-V Manager"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/win2012capture3.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "wab_hv-12"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }
}

BOOST_AUTO_TEST_CASE(TestNewOCRWin2012_VNC)
{
    Drawable drawable(1023, 768);

    using ocr::locale::LocaleId;
    OcrTitlesExtractor extractor(true, 100, LocaleId{LocaleId::latin});

    OcrTitleFilter filter;
    std::vector<OcrTitle> out_titles;

    {
        draw_bitmap(drawable, "./tests/fixtures/win2012_vnc_t9209.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "Bibliothèques"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }
}

BOOST_AUTO_TEST_CASE(TestNewOCRWin2012Standard)
{
    Drawable drawable(2880, 900);

    using ocr::locale::LocaleId;
    OcrTitlesExtractor extractor(true, 100, LocaleId{LocaleId::latin});

    OcrTitleFilter filter;
    std::vector<OcrTitle> out_titles;

    {
        draw_bitmap(drawable, "./tests/fixtures/w2012s_Gestionnaire_de_serveur.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
                "Gestionnaire de serveur"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/w2012s_Gestionnaire_des_taches.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
                "Gestionnaire des tàches"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/w2012s_Panneau_de_configuration.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
                "Panneau de configuration"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }
}


BOOST_AUTO_TEST_CASE(TestOCRBug)
{
    Drawable drawable(1364, 768);

    using ocr::locale::LocaleId;
    OcrTitlesExtractor extractor(true, 100, LocaleId{LocaleId::latin});

    OcrTitleFilter filter;
    std::vector<OcrTitle> out_titles;

    {
        draw_bitmap(drawable, "./tests/fixtures/badocr.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "Computer"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());
    }
}


BOOST_AUTO_TEST_CASE(TestNewOCRRussian)
{
    Drawable drawable(816, 639);

    using ocr::locale::LocaleId;
    OcrTitlesExtractor extractor(true, 100, LocaleId{LocaleId::cyrillic});

    OcrTitleFilter filter;
    std::vector<OcrTitle> out_titles;

    {
        draw_bitmap(drawable, "./tests/fixtures/win_unknow_russian.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "Устройства и принтеры"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }

    {
        draw_bitmap(drawable, "./tests/fixtures/win_unknow_russian2.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "Устройства и принтеры"
        );
        BOOST_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        BOOST_CHECK_EQUAL(idx_best, 0);
        BOOST_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }
}