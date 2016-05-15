/*
 * Copyright © 2012 Canonical Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authored By: Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#include <cstring>

#include <tr1/tuple>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>

#include <glib-object.h>

#include <gio/gio.h>

#include <glib_gslice_off_env.h>
#include <glib_gsettings_memory_backend_env.h>

#include <gtest_unspecified_bool_type_matcher.h>

#include "compiz_gwd_tests.h"

#include "gwd-settings.h"
#include "gwd-settings-storage.h"
#include "gwd-settings-writable-interface.h"

#include "decoration.h"

#include "compiz_gwd_mock_settings_writable.h"

using ::testing::TestWithParam;
using ::testing::Eq;
using ::testing::Return;
using ::testing::InvokeWithoutArgs;
using ::testing::IgnoreResult;
using ::testing::MatcherInterface;
using ::testing::MakeMatcher;
using ::testing::MatchResultListener;
using ::testing::Matcher;
using ::testing::Action;
using ::testing::ActionInterface;
using ::testing::MakeAction;
using ::testing::IsNull;
using ::testing::Values;
using ::testing::_;
using ::testing::StrictMock;
using ::testing::InSequence;

template <class ValueCType>
class GValueCmp
{
    public:

	typedef ValueCType (*GetFunc) (const GValue *value);

	bool compare (const ValueCType &val,
		      GValue           *value,
		      GetFunc	       get)
	{
	    const ValueCType &valForValue = (*get) (value);
	    return valForValue == val;
	}
};

template <>
class GValueCmp <decor_shadow_options_t>
{
    public:

	typedef gpointer (*GetFunc) (const GValue *value);

	bool compare (const decor_shadow_options_t &val,
		      GValue			   *value,
		      GetFunc			   get)
	{
	    gpointer shadowOptionsPtr = (*get) (value);
	    const decor_shadow_options_t &shadowOptions = *(reinterpret_cast <decor_shadow_options_t *> (shadowOptionsPtr));
	    if (decor_shadow_options_cmp (&val, &shadowOptions))
		return true;
	    else
		return false;
	}
};

template <>
class GValueCmp <std::string>
{
    public:

	typedef const gchar * (*GetFunc) (const GValue *value);

	bool compare (const std::string &val,
		      GValue	        *value,
		      GetFunc		get)
	{
	    const gchar *valueForValue = (*get) (value);
	    const std::string valueForValueStr (valueForValue);\

	    return val == valueForValueStr;
	}
};

namespace
{
    std::ostream &
    operator<< (std::ostream &os, const decor_shadow_options_t &options)
    {
	os << " radius: " << options.shadow_radius <<
	      " opacity: " << options.shadow_opacity <<
	      " offset: (" << options.shadow_offset_x << ", " << options.shadow_offset_y << ")" <<
	      " color: r: " << options.shadow_color[0] <<
	      " g: " << options.shadow_color[1] <<
	      " b: " << options.shadow_color[2];

	return os;
    }
}

template <class ValueCType>
class GObjectPropertyMatcher :
    public ::testing::MatcherInterface <GValue *>
{
    public:

	GObjectPropertyMatcher (const ValueCType			&value,
				typename GValueCmp<ValueCType>::GetFunc	func) :
	    mValue (value),
	    mGetFunc (func)
	{
	}
	virtual ~GObjectPropertyMatcher () {}

	virtual bool MatchAndExplain (GValue *value, MatchResultListener *listener) const
	{
	    return GValueCmp <ValueCType> ().compare (mValue, value, mGetFunc);
	}

	virtual void DescribeTo (std::ostream *os) const
	{
	    *os << "value contains " << mValue;
	}

	virtual void DescribeNegationTo (std::ostream *os) const
	{
	    *os << "value does not contain " << mValue;
	}

    private:

	const ValueCType &mValue;
	typename GValueCmp<ValueCType>::GetFunc mGetFunc;
};

namespace testing_values
{
    const gdouble ACTIVE_SHADOW_OPACITY_VALUE = 1.0;
    const gdouble ACTIVE_SHADOW_RADIUS_VALUE = 2.0;
    const gdouble ACTIVE_SHADOW_OFFSET_X_VALUE = 3.0;
    const gint    ACTIVE_SHADOW_OFFSET_X_INT_VALUE = ACTIVE_SHADOW_OFFSET_X_VALUE;
    const gdouble ACTIVE_SHADOW_OFFSET_Y_VALUE = 4.0;
    const gint    ACTIVE_SHADOW_OFFSET_Y_INT_VALUE = ACTIVE_SHADOW_OFFSET_Y_VALUE;
    const std::string ACTIVE_SHADOW_COLOR_STR_VALUE ("#ffffffff");
    const gushort ACTIVE_SHADOW_COLOR_VALUE[] = { 255, 255, 255 };
    const gdouble INACTIVE_SHADOW_OPACITY_VALUE = 5.0;
    const gdouble INACTIVE_SHADOW_RADIUS_VALUE = 6.0;
    const gdouble INACTIVE_SHADOW_OFFSET_X_VALUE = 7.0;
    const gint    INACTIVE_SHADOW_OFFSET_X_INT_VALUE = INACTIVE_SHADOW_OFFSET_X_VALUE;
    const gdouble INACTIVE_SHADOW_OFFSET_Y_VALUE = 8.0;
    const gint    INACTIVE_SHADOW_OFFSET_Y_INT_VALUE = INACTIVE_SHADOW_OFFSET_Y_VALUE;
    const std::string INACTIVE_SHADOW_COLOR_STR_VALUE ("#00000000");
    const gushort INACTIVE_SHADOW_COLOR_VALUE[] = { 0, 0, 0 };
    const gboolean USE_TOOLTIPS_VALUE = !USE_TOOLTIPS_DEFAULT;
    const std::string BLUR_TYPE_TITLEBAR_VALUE ("titlebar");
    const gint BLUR_TYPE_TITLEBAR_INT_VALUE = BLUR_TYPE_TITLEBAR;
    const std::string BLUR_TYPE_ALL_VALUE ("all");
    const gint BLUR_TYPE_ALL_INT_VALUE = BLUR_TYPE_ALL;
    const std::string BLUR_TYPE_NONE_VALUE ("none");
    const gint BLUR_TYPE_NONE_INT_VALUE = BLUR_TYPE_NONE;
    const gboolean USE_METACITY_THEME_VALUE  = TRUE;
    const std::string METACITY_THEME_VALUE ("metacity_theme");
    const gboolean NO_USE_METACITY_THEME_VALUE  = FALSE;
    const std::string NO_METACITY_THEME_VALUE ("");
    const gdouble ACTIVE_OPACITY_VALUE = 0.9;
    const gdouble INACTIVE_OPACITY_VALUE = 0.8;
    const gboolean ACTIVE_SHADE_OPACITY_VALUE = !METACITY_ACTIVE_SHADE_OPACITY_DEFAULT;
    const gboolean INACTIVE_SHADE_OPACITY_VALUE = !METACITY_INACTIVE_SHADE_OPACITY_DEFAULT;
    const std::string BUTTON_LAYOUT_VALUE ("button_layout");
    const gboolean USE_SYSTEM_FONT_VALUE = TRUE;
    const gboolean NO_USE_SYSTEM_FONT_VALUE = FALSE;
    const std::string TITLEBAR_FONT_VALUE ("Ubuntu 12");
    const std::string TITLEBAR_ACTION_SHADE ("toggle_shade");
    const std::string TITLEBAR_ACTION_MAX_VERT ("toggle_maximize_vertically");
    const std::string TITLEBAR_ACTION_MAX_HORZ ("toggle_maximize_horizontally");
    const std::string TITLEBAR_ACTION_MAX ("toggle_maximize");
    const std::string TITLEBAR_ACTION_MINIMIZE ("minimize");
    const std::string TITLEBAR_ACTION_MENU ("menu");
    const std::string TITLEBAR_ACTION_LOWER ("lower");
    const std::string TITLEBAR_ACTION_NONE ("none");
    const std::string MOUSE_WHEEL_ACTION_SHADE ("shade");
}

template <class ValueCType>
inline Matcher<GValue *>
GValueMatch (const ValueCType &value,
	     typename GValueCmp<ValueCType>::GetFunc	func)
{
    return MakeMatcher (new GObjectPropertyMatcher <ValueCType> (value, func));
}

class GWDSettingsTestCommon :
    public ::testing::Test
{
    public:
	virtual void SetUp ()
	{
	    env.SetUpEnv ();
	}
	virtual void TearDown ()
	{
	    env.TearDownEnv ();
	}
    private:

	CompizGLibGSliceOffEnv env;
};

class GWDMockSettingsWritableTest :
    public GWDSettingsTestCommon
{
};

namespace
{
    void gwd_settings_storage_unref (GWDSettingsStorage *storage)
    {
	g_object_unref (G_OBJECT (storage));
    }

    void gwd_settings_writable_unref (GWDSettingsWritable *writable)
    {
	g_object_unref (G_OBJECT (writable));
    }

    void gwd_settings_unref (GWDSettings *settings)
    {
	g_object_unref (G_OBJECT (settings));
    }

    /*void gwd_settings_notified_do_nothing (GWDSettingsNotified *notified)
    {
    }*/

    class AutoUnsetGValue
    {
	public:

	    AutoUnsetGValue (GType type)
	    {
		/* This is effectively G_VALUE_INIT, we can't use that here
		 * because this is not a C++11 project */
		mValue.g_type = 0;
		mValue.data[0].v_int = 0;
		mValue.data[1].v_int = 0;
		g_value_init (&mValue, type);
	    }

	    ~AutoUnsetGValue ()
	    {
		g_value_unset (&mValue);
	    }

	    operator GValue & ()
	    {
		return mValue;
	    }

	private:

	    GValue mValue;
    };
}

TEST_F(GWDMockSettingsWritableTest, TestMock)
{
    GWDMockSettingsWritableGMock writableGMock;
    boost::shared_ptr <GWDSettingsWritable> writableMock (gwd_mock_settings_writable_new (&writableGMock),
							  boost::bind (gwd_settings_writable_unref, _1));


    EXPECT_CALL (writableGMock, freezeUpdates ());
    EXPECT_CALL (writableGMock, thawUpdates ());
    EXPECT_CALL (writableGMock, shadowPropertyChanged (testing_values::ACTIVE_SHADOW_RADIUS_VALUE,
						       testing_values::ACTIVE_SHADOW_OPACITY_VALUE,
						       testing_values::ACTIVE_SHADOW_OFFSET_X_VALUE,
						       testing_values::ACTIVE_SHADOW_OFFSET_Y_VALUE,
						       Eq (testing_values::ACTIVE_SHADOW_COLOR_STR_VALUE),
						       testing_values::INACTIVE_SHADOW_RADIUS_VALUE,
						       testing_values::INACTIVE_SHADOW_OPACITY_VALUE,
						       testing_values::INACTIVE_SHADOW_OFFSET_X_VALUE,
						       testing_values::INACTIVE_SHADOW_OFFSET_Y_VALUE,
						       Eq (testing_values::INACTIVE_SHADOW_COLOR_STR_VALUE))).WillOnce (Return (TRUE));
    EXPECT_CALL (writableGMock, useTooltipsChanged (testing_values::USE_TOOLTIPS_VALUE)).WillOnce (Return (TRUE));
    EXPECT_CALL (writableGMock, blurChanged (Eq (testing_values::BLUR_TYPE_TITLEBAR_VALUE))).WillOnce (Return (TRUE));
    EXPECT_CALL (writableGMock, metacityThemeChanged (TRUE, Eq (testing_values::METACITY_THEME_VALUE))).WillOnce (Return (TRUE));
    EXPECT_CALL (writableGMock, opacityChanged (testing_values::ACTIVE_OPACITY_VALUE,
						testing_values::INACTIVE_OPACITY_VALUE,
						testing_values::ACTIVE_SHADE_OPACITY_VALUE,
						testing_values::INACTIVE_SHADE_OPACITY_VALUE)).WillOnce (Return (TRUE));
    EXPECT_CALL (writableGMock, buttonLayoutChanged (Eq (testing_values::BUTTON_LAYOUT_VALUE))).WillOnce (Return (TRUE));
    EXPECT_CALL (writableGMock, fontChanged (testing_values::USE_SYSTEM_FONT_VALUE,
					     testing_values::TITLEBAR_FONT_VALUE.c_str ())).WillOnce (Return (TRUE));
    EXPECT_CALL (writableGMock, titlebarActionsChanged (Eq (testing_values::TITLEBAR_ACTION_MAX),
							Eq (testing_values::TITLEBAR_ACTION_MENU),
							Eq (testing_values::TITLEBAR_ACTION_LOWER),
							Eq (testing_values::TITLEBAR_ACTION_SHADE))).WillOnce (Return (TRUE));

    EXPECT_CALL (writableGMock, dispose ());
    EXPECT_CALL (writableGMock, finalize ());

    gwd_settings_writable_freeze_updates (writableMock.get ());
    gwd_settings_writable_thaw_updates (writableMock.get ());

    EXPECT_THAT (gwd_settings_writable_shadow_property_changed (writableMock.get (),
								testing_values::ACTIVE_SHADOW_RADIUS_VALUE,
								testing_values::ACTIVE_SHADOW_OPACITY_VALUE,
								testing_values::ACTIVE_SHADOW_OFFSET_X_VALUE,
								testing_values::ACTIVE_SHADOW_OFFSET_Y_VALUE,
								testing_values::ACTIVE_SHADOW_COLOR_STR_VALUE.c_str (),
								testing_values::INACTIVE_SHADOW_RADIUS_VALUE,
								testing_values::INACTIVE_SHADOW_OPACITY_VALUE,
								testing_values::INACTIVE_SHADOW_OFFSET_X_VALUE,
								testing_values::INACTIVE_SHADOW_OFFSET_Y_VALUE,
								testing_values::INACTIVE_SHADOW_COLOR_STR_VALUE.c_str ()), IsTrue ());
    EXPECT_THAT (gwd_settings_writable_use_tooltips_changed (writableMock.get (), testing_values::USE_TOOLTIPS_VALUE), IsTrue ());
    EXPECT_THAT (gwd_settings_writable_blur_changed (writableMock.get (), testing_values::BLUR_TYPE_TITLEBAR_VALUE.c_str ()), IsTrue ());
    EXPECT_THAT (gwd_settings_writable_metacity_theme_changed (writableMock.get (),
							       testing_values::USE_METACITY_THEME_VALUE,
							       testing_values::METACITY_THEME_VALUE.c_str ()), IsTrue ());
    EXPECT_THAT (gwd_settings_writable_opacity_changed (writableMock.get (),
							testing_values::ACTIVE_OPACITY_VALUE,
							testing_values::INACTIVE_OPACITY_VALUE,
							testing_values::ACTIVE_SHADE_OPACITY_VALUE,
							testing_values::INACTIVE_SHADE_OPACITY_VALUE), IsTrue ());
    EXPECT_THAT (gwd_settings_writable_button_layout_changed (writableMock.get (),
							      testing_values::BUTTON_LAYOUT_VALUE.c_str ()), IsTrue ());
    EXPECT_THAT (gwd_settings_writable_font_changed (writableMock.get (),
						     testing_values::USE_SYSTEM_FONT_VALUE,
						     testing_values::TITLEBAR_FONT_VALUE.c_str ()), IsTrue ());
    EXPECT_THAT (gwd_settings_writable_titlebar_actions_changed (writableMock.get (),
								 testing_values::TITLEBAR_ACTION_MAX.c_str (),
								 testing_values::TITLEBAR_ACTION_MENU.c_str (),
								 testing_values::TITLEBAR_ACTION_LOWER.c_str (),
								 testing_values::TITLEBAR_ACTION_SHADE.c_str ()), IsTrue ());
}

/*namespace
{
    void ExpectAllNotificationsOnce (boost::shared_ptr <StrictMock <GWDMockSettingsNotifiedGMock> > &gmockNotified,
				     boost::shared_ptr <GWDSettings> &settings)
    {
	InSequence s;

	EXPECT_CALL (*gmockNotified, updateMetacityTheme ()).Times (1);
	EXPECT_CALL (*gmockNotified, updateMetacityButtonLayout ()).Times (1);
	EXPECT_CALL (*gmockNotified, updateFrames ()).Times (1);
	EXPECT_CALL (*gmockNotified, updateDecorations ()).Times (1);

	gwd_settings_writable_thaw_updates (GWD_SETTINGS_WRITABLE_INTERFACE (settings.get ()));
    }
}*/

class GWDSettingsTest :
    public GWDSettingsTestCommon
{
    public:

	virtual void SetUp ()
	{
	    GWDSettingsTestCommon::SetUp ();
	    /*mGMockNotified.reset (new StrictMock <GWDMockSettingsNotifiedGMock> ());
	    mMockNotified.reset (gwd_mock_settings_notified_new (mGMockNotified.get ()),
				 boost::bind (gwd_settings_notified_do_nothing, _1));*/
	    mSettings.reset (gwd_settings_new (NULL, NULL),
			     boost::bind (gwd_settings_unref, _1));
	    /*ExpectAllNotificationsOnce (mGMockNotified, mSettings);*/
	}

	virtual void TearDown ()
	{
	    /*EXPECT_CALL (*mGMockNotified, dispose ());
	    EXPECT_CALL (*mGMockNotified, finalize ());*/
	}

    protected:

	/*boost::shared_ptr <StrictMock <GWDMockSettingsNotifiedGMock> > mGMockNotified;
	boost::shared_ptr <GWDSettingsNotified> mMockNotified;*/
	boost::shared_ptr <GWDSettings> mSettings;
};

TEST_F(GWDSettingsTest, TestGWDSettingsInstantiation)
{
}

/* Won't fail if the code in SetUp succeeds */
TEST_F(GWDSettingsTest, TestUpdateAllOnInstantiation)
{
}

/* We're just using use_tooltips here as an example */
TEST_F(GWDSettingsTest, TestFreezeUpdatesNoUpdates)
{
    gwd_settings_writable_freeze_updates (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()));
    EXPECT_THAT (gwd_settings_writable_use_tooltips_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							     testing_values::USE_TOOLTIPS_VALUE), IsTrue ());
}

/* We're just using use_tooltips here as an example */
TEST_F(GWDSettingsTest, TestFreezeUpdatesNoUpdatesThawUpdatesAllUpdates)
{
    gwd_settings_writable_freeze_updates (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()));
    EXPECT_THAT (gwd_settings_writable_use_tooltips_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							     testing_values::USE_TOOLTIPS_VALUE), IsTrue ());

    /*EXPECT_CALL (*mGMockNotified, updateDecorations ());*/
    gwd_settings_writable_thaw_updates (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()));
}

/* We're just using use_tooltips here as an example */
TEST_F(GWDSettingsTest, TestFreezeUpdatesNoUpdatesThawUpdatesAllUpdatesNoDupes)
{
    gwd_settings_writable_freeze_updates (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()));
    EXPECT_THAT (gwd_settings_writable_use_tooltips_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							     testing_values::USE_TOOLTIPS_VALUE), IsTrue ());
    EXPECT_THAT (gwd_settings_writable_use_tooltips_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							     !testing_values::USE_TOOLTIPS_VALUE), IsTrue ());
    EXPECT_THAT (gwd_settings_writable_use_tooltips_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							     testing_values::USE_TOOLTIPS_VALUE), IsTrue ());

    /*EXPECT_CALL (*mGMockNotified, updateDecorations ()).Times (1);*/
    gwd_settings_writable_thaw_updates (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()));
}

TEST_F(GWDSettingsTest, TestShadowPropertyChanged)
{
    /*EXPECT_CALL (*mGMockNotified, updateDecorations ());*/
    EXPECT_THAT (gwd_settings_writable_shadow_property_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
								testing_values::ACTIVE_SHADOW_OPACITY_VALUE,
								testing_values::ACTIVE_SHADOW_RADIUS_VALUE,
								testing_values::ACTIVE_SHADOW_OFFSET_X_VALUE,
								testing_values::ACTIVE_SHADOW_OFFSET_Y_VALUE,
								testing_values::ACTIVE_SHADOW_COLOR_STR_VALUE.c_str (),
								testing_values::INACTIVE_SHADOW_OPACITY_VALUE,
								testing_values::INACTIVE_SHADOW_RADIUS_VALUE,
								testing_values::INACTIVE_SHADOW_OFFSET_X_VALUE,
								testing_values::INACTIVE_SHADOW_OFFSET_Y_VALUE,
								testing_values::INACTIVE_SHADOW_COLOR_STR_VALUE.c_str ()), IsTrue ());

    AutoUnsetGValue activeShadowValue (G_TYPE_POINTER);
    AutoUnsetGValue inactiveShadowValue (G_TYPE_POINTER);

    GValue &activeShadowGValue = activeShadowValue;
    GValue &inactiveShadowGValue = inactiveShadowValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "active-shadow",
			   &activeShadowGValue);

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "inactive-shadow",
			   &inactiveShadowGValue);

    decor_shadow_options_t activeShadow;

    activeShadow.shadow_opacity = testing_values::ACTIVE_SHADOW_OPACITY_VALUE;
    activeShadow.shadow_radius = testing_values::ACTIVE_SHADOW_RADIUS_VALUE;
    activeShadow.shadow_offset_x = testing_values::ACTIVE_SHADOW_OFFSET_X_INT_VALUE;
    activeShadow.shadow_offset_y = testing_values::ACTIVE_SHADOW_OFFSET_Y_INT_VALUE;
    activeShadow.shadow_color[0] = testing_values::ACTIVE_SHADOW_COLOR_VALUE[0];
    activeShadow.shadow_color[1] = testing_values::ACTIVE_SHADOW_COLOR_VALUE[1];
    activeShadow.shadow_color[2] = testing_values::ACTIVE_SHADOW_COLOR_VALUE[2];

    decor_shadow_options_t inactiveShadow;

    inactiveShadow.shadow_opacity = testing_values::INACTIVE_SHADOW_OPACITY_VALUE;
    inactiveShadow.shadow_radius = testing_values::INACTIVE_SHADOW_RADIUS_VALUE;
    inactiveShadow.shadow_offset_x = testing_values::INACTIVE_SHADOW_OFFSET_X_INT_VALUE;
    inactiveShadow.shadow_offset_y = testing_values::INACTIVE_SHADOW_OFFSET_Y_INT_VALUE;
    inactiveShadow.shadow_color[0] = testing_values::INACTIVE_SHADOW_COLOR_VALUE[0];
    inactiveShadow.shadow_color[1] = testing_values::INACTIVE_SHADOW_COLOR_VALUE[1];
    inactiveShadow.shadow_color[2] = testing_values::INACTIVE_SHADOW_COLOR_VALUE[2];

    EXPECT_THAT (&activeShadowGValue, GValueMatch <decor_shadow_options_t> (activeShadow,
									    g_value_get_pointer));
    EXPECT_THAT (&inactiveShadowGValue, GValueMatch <decor_shadow_options_t> (inactiveShadow,
									      g_value_get_pointer));
}

TEST_F(GWDSettingsTest, TestShadowPropertyChangedIsDefault)
{
    EXPECT_THAT (gwd_settings_writable_shadow_property_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
								ACTIVE_SHADOW_RADIUS_DEFAULT,
								ACTIVE_SHADOW_OPACITY_DEFAULT,
								ACTIVE_SHADOW_OFFSET_X_DEFAULT,
								ACTIVE_SHADOW_OFFSET_Y_DEFAULT,
								ACTIVE_SHADOW_COLOR_DEFAULT,
								INACTIVE_SHADOW_RADIUS_DEFAULT,
								INACTIVE_SHADOW_OPACITY_DEFAULT,
								INACTIVE_SHADOW_OFFSET_X_DEFAULT,
								INACTIVE_SHADOW_OFFSET_Y_DEFAULT,
								INACTIVE_SHADOW_COLOR_DEFAULT), IsFalse ());
}

TEST_F(GWDSettingsTest, TestUseTooltipsChanged)
{
    /*EXPECT_CALL (*mGMockNotified, updateDecorations ());*/
    EXPECT_THAT (gwd_settings_writable_use_tooltips_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							     testing_values::USE_TOOLTIPS_VALUE), IsTrue ());

    AutoUnsetGValue useTooltipsValue (G_TYPE_BOOLEAN);
    GValue &useTooltipsGValue = useTooltipsValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "use-tooltips",
			   &useTooltipsGValue);

    EXPECT_THAT (&useTooltipsGValue, GValueMatch <gboolean> (testing_values::USE_TOOLTIPS_VALUE,
							     g_value_get_boolean));
}

TEST_F(GWDSettingsTest, TestUseTooltipsChangedIsDefault)
{
    EXPECT_THAT (gwd_settings_writable_use_tooltips_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							     USE_TOOLTIPS_DEFAULT), IsFalse ());
}

TEST_F(GWDSettingsTest, TestBlurChangedTitlebar)
{
    /*EXPECT_CALL (*mGMockNotified, updateDecorations ());*/
    EXPECT_THAT (gwd_settings_writable_blur_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
						     testing_values::BLUR_TYPE_TITLEBAR_VALUE.c_str ()), IsTrue ());

    AutoUnsetGValue blurValue (G_TYPE_INT);
    GValue &blurGValue = blurValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "blur",
			   &blurGValue);

    EXPECT_THAT (&blurGValue, GValueMatch <gint> (testing_values::BLUR_TYPE_TITLEBAR_INT_VALUE,
						  g_value_get_int));
}

TEST_F(GWDSettingsTest, TestBlurChangedAll)
{
    /*EXPECT_CALL (*mGMockNotified, updateDecorations ());*/
    EXPECT_THAT (gwd_settings_writable_blur_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
						     testing_values::BLUR_TYPE_ALL_VALUE.c_str ()), IsTrue ());

    AutoUnsetGValue blurValue (G_TYPE_INT);
    GValue &blurGValue = blurValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "blur",
			   &blurGValue);

    EXPECT_THAT (&blurGValue, GValueMatch <gint> (testing_values::BLUR_TYPE_ALL_INT_VALUE,
						  g_value_get_int));
}

TEST_F(GWDSettingsTest, TestBlurChangedNone)
{
    EXPECT_THAT (gwd_settings_writable_blur_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
						     testing_values::BLUR_TYPE_NONE_VALUE.c_str ()), IsFalse ());

    AutoUnsetGValue blurValue (G_TYPE_INT);
    GValue &blurGValue = blurValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "blur",
			   &blurGValue);

    EXPECT_THAT (&blurGValue, GValueMatch <gint> (testing_values::BLUR_TYPE_NONE_INT_VALUE,
						  g_value_get_int));
}

TEST_F(GWDSettingsTest, TestBlurSetCommandLine)
{
    gint blurType = testing_values::BLUR_TYPE_ALL_INT_VALUE;

    mSettings.reset (gwd_settings_new (&blurType, NULL),
		     boost::bind (gwd_settings_unref, _1));

    EXPECT_THAT (gwd_settings_writable_blur_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
						     testing_values::BLUR_TYPE_NONE_VALUE.c_str ()), IsFalse ());

    AutoUnsetGValue blurValue (G_TYPE_INT);
    GValue &blurGValue = blurValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "blur",
			   &blurGValue);

    EXPECT_THAT (&blurGValue, GValueMatch <gint> (testing_values::BLUR_TYPE_ALL_INT_VALUE,
						  g_value_get_int));
}

TEST_F(GWDSettingsTest, TestMetacityThemeChanged)
{
    /*EXPECT_CALL (*mGMockNotified, updateMetacityTheme ());
    EXPECT_CALL (*mGMockNotified, updateDecorations ());*/
    EXPECT_THAT (gwd_settings_writable_metacity_theme_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							       testing_values::USE_METACITY_THEME_VALUE,
							       testing_values::METACITY_THEME_VALUE.c_str ()), IsTrue ());

    AutoUnsetGValue metacityThemeValue (G_TYPE_STRING);
    GValue &metacityThemeGValue = metacityThemeValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "metacity-theme",
			   &metacityThemeGValue);

    EXPECT_THAT (&metacityThemeGValue, GValueMatch <std::string> (testing_values::METACITY_THEME_VALUE,
								  g_value_get_string));
}

TEST_F(GWDSettingsTest, TestMetacityThemeChangedNoUseMetacityTheme)
{
    /*EXPECT_CALL (*mGMockNotified, updateMetacityTheme ());
    EXPECT_CALL (*mGMockNotified, updateDecorations ());*/
    EXPECT_THAT (gwd_settings_writable_metacity_theme_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							       testing_values::NO_USE_METACITY_THEME_VALUE,
							       testing_values::METACITY_THEME_VALUE.c_str ()), IsTrue ());

    AutoUnsetGValue metacityThemeValue (G_TYPE_STRING);
    GValue &metacityThemeGValue = metacityThemeValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "metacity-theme",
			   &metacityThemeGValue);

    EXPECT_THAT (&metacityThemeGValue, GValueMatch <std::string> (testing_values::NO_METACITY_THEME_VALUE,
								  g_value_get_string));
}

TEST_F(GWDSettingsTest, TestMetacityThemeChangedIsDefault)
{
    EXPECT_THAT (gwd_settings_writable_metacity_theme_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							       testing_values::USE_METACITY_THEME_VALUE,
							       METACITY_THEME_DEFAULT), IsFalse ());
}

TEST_F(GWDSettingsTest, TestMetacityThemeSetCommandLine)
{
    const gchar *metacityTheme = "Ambiance";

    mSettings.reset (gwd_settings_new (NULL, &metacityTheme),
		     boost::bind (gwd_settings_unref, _1));

    EXPECT_THAT (gwd_settings_writable_metacity_theme_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							       testing_values::USE_METACITY_THEME_VALUE,
							       testing_values::METACITY_THEME_VALUE.c_str ()), IsFalse ());

    AutoUnsetGValue metacityThemeValue (G_TYPE_STRING);
    GValue &metacityThemeGValue = metacityThemeValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "metacity-theme",
			   &metacityThemeGValue);

    EXPECT_THAT (&metacityThemeGValue, GValueMatch <std::string> (std::string (metacityTheme),
								  g_value_get_string));
}

TEST_F(GWDSettingsTest, TestMetacityOpacityChanged)
{
    /*EXPECT_CALL (*mGMockNotified, updateDecorations ());*/
    EXPECT_THAT (gwd_settings_writable_opacity_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							testing_values::ACTIVE_OPACITY_VALUE,
							testing_values::INACTIVE_OPACITY_VALUE,
							testing_values::ACTIVE_SHADE_OPACITY_VALUE,
							testing_values::INACTIVE_SHADE_OPACITY_VALUE), IsTrue ());

    AutoUnsetGValue metacityInactiveOpacityValue (G_TYPE_DOUBLE);
    AutoUnsetGValue metacityActiveOpacityValue (G_TYPE_DOUBLE);
    AutoUnsetGValue metacityInactiveShadeOpacityValue (G_TYPE_BOOLEAN);
    AutoUnsetGValue metacityActiveShadeOpacityValue (G_TYPE_BOOLEAN);

    GValue &metacityInactiveOpacityGValue = metacityInactiveOpacityValue;
    GValue &metacityActiveOpacityGValue = metacityActiveOpacityValue;
    GValue &metacityInactiveShadeOpacityGValue = metacityInactiveShadeOpacityValue;
    GValue &metacityActiveShadeOpacityGValue = metacityActiveShadeOpacityValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "metacity-inactive-opacity",
			   &metacityInactiveOpacityGValue);
    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "metacity-active-opacity",
			   &metacityActiveOpacityGValue);
    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "metacity-inactive-shade-opacity",
			   &metacityInactiveShadeOpacityGValue);
    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "metacity-active-shade-opacity",
			   &metacityActiveShadeOpacityGValue);

    EXPECT_THAT (&metacityInactiveOpacityGValue, GValueMatch <gdouble> (testing_values::INACTIVE_OPACITY_VALUE,
									g_value_get_double));
    EXPECT_THAT (&metacityActiveOpacityGValue, GValueMatch <gdouble> (testing_values::ACTIVE_OPACITY_VALUE,
									g_value_get_double));
    EXPECT_THAT (&metacityInactiveShadeOpacityGValue, GValueMatch <gboolean> (testing_values::INACTIVE_SHADE_OPACITY_VALUE,
									g_value_get_boolean));
    EXPECT_THAT (&metacityActiveShadeOpacityGValue, GValueMatch <gboolean> (testing_values::ACTIVE_SHADE_OPACITY_VALUE,
									g_value_get_boolean));
}

TEST_F(GWDSettingsTest, TestMetacityOpacityChangedIsDefault)
{
    EXPECT_THAT (gwd_settings_writable_opacity_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							METACITY_ACTIVE_OPACITY_DEFAULT,
							METACITY_INACTIVE_OPACITY_DEFAULT,
							METACITY_ACTIVE_SHADE_OPACITY_DEFAULT,
							METACITY_INACTIVE_SHADE_OPACITY_DEFAULT), IsFalse ());
}

TEST_F(GWDSettingsTest, TestButtonLayoutChanged)
{
    /*EXPECT_CALL (*mGMockNotified, updateMetacityButtonLayout ());
    EXPECT_CALL (*mGMockNotified, updateDecorations ());*/
    EXPECT_THAT (gwd_settings_writable_button_layout_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							      testing_values::BUTTON_LAYOUT_VALUE.c_str ()), IsTrue ());

    AutoUnsetGValue buttonLayoutValue (G_TYPE_STRING);
    GValue &buttonLayoutGValue = buttonLayoutValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "metacity-button-layout",
			   &buttonLayoutGValue);

    EXPECT_THAT (&buttonLayoutGValue, GValueMatch <std::string> (testing_values::BUTTON_LAYOUT_VALUE,
								 g_value_get_string));
}

TEST_F(GWDSettingsTest, TestButtonLayoutChangedIsDefault)
{
    EXPECT_THAT (gwd_settings_writable_button_layout_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
							      METACITY_BUTTON_LAYOUT_DEFAULT), IsFalse ());
}

TEST_F(GWDSettingsTest, TestTitlebarFontChanged)
{
    /*EXPECT_CALL (*mGMockNotified, updateFrames ());
    EXPECT_CALL (*mGMockNotified, updateDecorations ());*/
    EXPECT_THAT (gwd_settings_writable_font_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
						     testing_values::NO_USE_SYSTEM_FONT_VALUE,
						     testing_values::TITLEBAR_FONT_VALUE.c_str ()), IsTrue ());

    AutoUnsetGValue fontValue (G_TYPE_STRING);
    GValue	    &fontGValue = fontValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "titlebar-font",
			   &fontGValue);

    EXPECT_THAT (&fontGValue, GValueMatch <std::string> (testing_values::TITLEBAR_FONT_VALUE.c_str (),
							 g_value_get_string));
}

TEST_F(GWDSettingsTest, TestTitlebarFontChangedUseSystemFont)
{
    /*EXPECT_CALL (*mGMockNotified, updateFrames ());
    EXPECT_CALL (*mGMockNotified, updateDecorations ());*/
    EXPECT_THAT (gwd_settings_writable_font_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
						     testing_values::USE_SYSTEM_FONT_VALUE,
						     testing_values::TITLEBAR_FONT_VALUE.c_str ()), IsTrue ());

    AutoUnsetGValue fontValue (G_TYPE_STRING);
    GValue	    &fontGValue = fontValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "titlebar-font",
			   &fontGValue);

    EXPECT_THAT (&fontGValue, GValueMatch <const gchar *> (NULL,
							   g_value_get_string));
}


TEST_F(GWDSettingsTest, TestTitlebarFontChangedIsDefault)
{
    EXPECT_THAT (gwd_settings_writable_font_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
						     testing_values::NO_USE_SYSTEM_FONT_VALUE,
						     TITLEBAR_FONT_DEFAULT), IsFalse ());
}

namespace
{
    class GWDTitlebarActionInfo
    {
	public:

	    GWDTitlebarActionInfo (const std::string &titlebarAction,
				   const std::string &mouseWheelAction,
				   const gint	     titlebarActionId,
				   const gint	     mouseWheelActionId) :
		mTitlebarAction (titlebarAction),
		mMouseWheelAction (mouseWheelAction),
		mTitlebarActionId (titlebarActionId),
		mMouseWheelActionId (mouseWheelActionId)
	    {
	    }

	    const std::string & titlebarAction () const { return mTitlebarAction; }
	    const std::string & mouseWheelAction () const { return mMouseWheelAction; }
	    const gint	      & titlebarActionId () const { return mTitlebarActionId; }
	    const gint	      & mouseWheelActionId () const { return mMouseWheelActionId; }

	private:

	    std::string mTitlebarAction;
	    std::string mMouseWheelAction;
	    gint	mTitlebarActionId;
	    gint	mMouseWheelActionId;
    };
}

class GWDSettingsTestClickActions :
    public GWDSettingsTestCommon,
    public ::testing::WithParamInterface <GWDTitlebarActionInfo>
{
    public:

	virtual void SetUp ()
	{
	    GWDSettingsTestCommon::SetUp ();
	    /*mGMockNotified.reset (new GWDMockSettingsNotifiedGMock ());
	    mMockNotified.reset (gwd_mock_settings_notified_new (mGMockNotified.get ()),
				 boost::bind (gwd_settings_notified_do_nothing, _1));*/
	    mSettings.reset (gwd_settings_new (NULL, NULL),
			     boost::bind (gwd_settings_unref, _1));
	}

	virtual void TearDown ()
	{
	    /*EXPECT_CALL (*mGMockNotified, dispose ());
	    EXPECT_CALL (*mGMockNotified, finalize ());*/

	    GWDSettingsTestCommon::TearDown ();
	}

    protected:

	/*boost::shared_ptr <GWDMockSettingsNotifiedGMock> mGMockNotified;
	boost::shared_ptr <GWDSettingsNotified> mMockNotified;*/
	boost::shared_ptr <GWDSettings> mSettings;
};

TEST_P(GWDSettingsTestClickActions, TestClickActionsAndMouseActions)
{
    gwd_settings_writable_titlebar_actions_changed (GWD_SETTINGS_WRITABLE_INTERFACE (mSettings.get ()),
						    GetParam ().titlebarAction ().c_str (),
						    GetParam ().titlebarAction ().c_str (),
						    GetParam ().titlebarAction ().c_str (),
						    GetParam ().mouseWheelAction ().c_str ());

    AutoUnsetGValue doubleClickActionValue (G_TYPE_INT);
    AutoUnsetGValue middleClickActionValue (G_TYPE_INT);
    AutoUnsetGValue rightClickActionValue (G_TYPE_INT);
    AutoUnsetGValue mouseWheelActionValue (G_TYPE_INT);

    GValue &doubleClickActionGValue = doubleClickActionValue;
    GValue &middleClickActionGValue = middleClickActionValue;
    GValue &rightClickActionGValue = rightClickActionValue;
    GValue &mouseWheelActionGValue = mouseWheelActionValue;

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "titlebar-double-click-action",
			   &doubleClickActionGValue);

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "titlebar-middle-click-action",
			   &middleClickActionGValue);

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "titlebar-right-click-action",
			   &rightClickActionGValue);

    g_object_get_property (G_OBJECT (mSettings.get ()),
			   "mouse-wheel-action",
			   &mouseWheelActionGValue);

    EXPECT_THAT (&doubleClickActionGValue, GValueMatch <gint> (GetParam ().titlebarActionId (),
							      g_value_get_int));
    EXPECT_THAT (&middleClickActionGValue, GValueMatch <gint> (GetParam ().titlebarActionId (),
							      g_value_get_int));
    EXPECT_THAT (&rightClickActionGValue, GValueMatch <gint> (GetParam ().titlebarActionId (),
							     g_value_get_int));
    EXPECT_THAT (&mouseWheelActionGValue, GValueMatch <gint> (GetParam ().mouseWheelActionId (),
							     g_value_get_int));
}

INSTANTIATE_TEST_CASE_P (MouseActions, GWDSettingsTestClickActions,
			 ::testing::Values (GWDTitlebarActionInfo (testing_values::TITLEBAR_ACTION_NONE,
								   testing_values::TITLEBAR_ACTION_NONE,
								   CLICK_ACTION_NONE,
								   WHEEL_ACTION_NONE),
					    GWDTitlebarActionInfo (testing_values::TITLEBAR_ACTION_SHADE,
								   testing_values::MOUSE_WHEEL_ACTION_SHADE,
								   CLICK_ACTION_SHADE,
								   WHEEL_ACTION_SHADE),
					    GWDTitlebarActionInfo (testing_values::TITLEBAR_ACTION_MAX,
								   testing_values::MOUSE_WHEEL_ACTION_SHADE,
								   CLICK_ACTION_MAXIMIZE,
								   WHEEL_ACTION_SHADE),
					    GWDTitlebarActionInfo (testing_values::TITLEBAR_ACTION_MINIMIZE,
								   testing_values::MOUSE_WHEEL_ACTION_SHADE,
								   CLICK_ACTION_MINIMIZE,
								   WHEEL_ACTION_SHADE),
					    GWDTitlebarActionInfo (testing_values::TITLEBAR_ACTION_LOWER,
								   testing_values::MOUSE_WHEEL_ACTION_SHADE,
								   CLICK_ACTION_LOWER,
								   WHEEL_ACTION_SHADE),
					    GWDTitlebarActionInfo (testing_values::TITLEBAR_ACTION_MENU,
								   testing_values::MOUSE_WHEEL_ACTION_SHADE,
								   CLICK_ACTION_MENU,
								   WHEEL_ACTION_SHADE)));

class GWDSettingsStorageFactoryWrapperInterface
{
    public:

	typedef boost::shared_ptr <GWDSettingsStorageFactoryWrapperInterface> Ptr;
	virtual ~GWDSettingsStorageFactoryWrapperInterface () {}

	virtual void SetUp (GWDSettingsWritable *writable) = 0;
	virtual GWDSettingsStorage * GetStorage () = 0;
	virtual void SetUseTooltips (gboolean useTooltips) = 0;
	virtual void SetBlur (const std::string &blurType) = 0;
	virtual void SetOpacity (gdouble activeOpacity,
				 gdouble inactiveOpacity,
				 gboolean activeShadeOpacity,
				 gboolean inactiveShadeOpacity) = 0;
	virtual void SetMetacityTheme (gboolean useMetacityTheme,
					  const std::string &metacityTheme) = 0;
	virtual void SetButtonLayout (const std::string &buttonLayout) = 0;
	virtual void SetFont (gboolean useSystemFont, const std::string &titlebarFont) = 0;
	virtual void SetTitlebarActions (const std::string &doubleClickAction,
					 const std::string &middleClickAction,
					 const std::string &rightClickAction,
					 const std::string &mouseWheelAction) = 0;
	virtual void TearDown () = 0;
};

class GWDSettingsTestStorageUpdates :
    public GWDSettingsTestCommon,
    public ::testing::WithParamInterface <GWDSettingsStorageFactoryWrapperInterface::Ptr>
{
    public:

	virtual void SetUp ()
	{
	    GWDSettingsTestCommon::SetUp ();
	    mSettingsMock.reset (new GWDMockSettingsWritableGMock ());
	    mSettings.reset (gwd_mock_settings_writable_new (mSettingsMock.get ()),
			     boost::bind (gwd_settings_writable_unref, _1));

	    GetParam ()->SetUp (mSettings.get ());
	}

	virtual void TearDown ()
	{
	    EXPECT_CALL (*mSettingsMock, dispose ());
	    EXPECT_CALL (*mSettingsMock, finalize ());

	    GetParam ()->TearDown ();
	    GWDSettingsTestCommon::TearDown ();
	}

    protected:

	boost::shared_ptr <GWDMockSettingsWritableGMock> mSettingsMock;
	boost::shared_ptr <GWDSettingsWritable> mSettings;
};

ACTION_P (InvokeFunctor, p) { return p (); }

TEST_P (GWDSettingsTestStorageUpdates, TestInstantiation)
{
}

TEST_P (GWDSettingsTestStorageUpdates, TestSetUseTooltips)
{
    GWDSettingsStorage *storage = GetParam ()->GetStorage ();
    GetParam ()->SetUseTooltips (testing_values::USE_TOOLTIPS_VALUE);

    EXPECT_CALL (*mSettingsMock, useTooltipsChanged (testing_values::USE_TOOLTIPS_VALUE));

    gwd_settings_storage_update_use_tooltips (storage);
}

TEST_P (GWDSettingsTestStorageUpdates, TestSetBlur)
{
    GWDSettingsStorage *storage = GetParam ()->GetStorage ();
    GetParam ()->SetBlur (testing_values::BLUR_TYPE_ALL_VALUE);

    EXPECT_CALL (*mSettingsMock, blurChanged (Eq (testing_values::BLUR_TYPE_ALL_VALUE)));

    gwd_settings_storage_update_blur (storage);
}

TEST_P (GWDSettingsTestStorageUpdates, TestSetButtonLayout)
{
    GWDSettingsStorage *storage = GetParam ()->GetStorage ();
    GetParam ()->SetButtonLayout (testing_values::BUTTON_LAYOUT_VALUE);

    EXPECT_CALL (*mSettingsMock, buttonLayoutChanged (Eq (testing_values::BUTTON_LAYOUT_VALUE)));

    gwd_settings_storage_update_button_layout (storage);
}

TEST_P (GWDSettingsTestStorageUpdates, TestSetOpacity)
{
    GWDSettingsStorage *storage = GetParam ()->GetStorage ();
    GetParam ()->SetOpacity (testing_values::ACTIVE_OPACITY_VALUE,
			     testing_values::INACTIVE_OPACITY_VALUE,
			     testing_values::ACTIVE_SHADE_OPACITY_VALUE,
			     testing_values::INACTIVE_SHADE_OPACITY_VALUE);

    EXPECT_CALL (*mSettingsMock, opacityChanged (testing_values::ACTIVE_OPACITY_VALUE,
						 testing_values::INACTIVE_OPACITY_VALUE,
						 testing_values::ACTIVE_SHADE_OPACITY_VALUE,
						 testing_values::INACTIVE_SHADE_OPACITY_VALUE));

    gwd_settings_storage_update_opacity (storage);
}

TEST_P (GWDSettingsTestStorageUpdates, TestSetMetacityTheme)
{
    GWDSettingsStorage *storage = GetParam ()->GetStorage ();
    GetParam ()->SetMetacityTheme (testing_values::USE_METACITY_THEME_VALUE,
				   testing_values::METACITY_THEME_VALUE);

    EXPECT_CALL (*mSettingsMock, metacityThemeChanged (testing_values::USE_METACITY_THEME_VALUE,
						       Eq (testing_values::METACITY_THEME_VALUE)));

    gwd_settings_storage_update_metacity_theme (storage);
}

TEST_P (GWDSettingsTestStorageUpdates, TestSetFont)
{
    GWDSettingsStorage *storage = GetParam ()->GetStorage ();
    GetParam ()->SetFont (testing_values::USE_SYSTEM_FONT_VALUE,
			  testing_values::TITLEBAR_FONT_VALUE);

    EXPECT_CALL (*mSettingsMock, fontChanged (testing_values::USE_SYSTEM_FONT_VALUE,
					      Eq (testing_values::TITLEBAR_FONT_VALUE)));

    gwd_settings_storage_update_font (storage);
}

TEST_P (GWDSettingsTestStorageUpdates, TestSetTitlebarActions)
{
    GWDSettingsStorage *storage = GetParam ()->GetStorage ();
    GetParam ()->SetTitlebarActions (testing_values::TITLEBAR_ACTION_LOWER,
				     testing_values::TITLEBAR_ACTION_MAX,
				     testing_values::TITLEBAR_ACTION_MENU,
				     testing_values::TITLEBAR_ACTION_SHADE);

    EXPECT_CALL (*mSettingsMock, titlebarActionsChanged (Eq (testing_values::TITLEBAR_ACTION_LOWER),
							 Eq (testing_values::TITLEBAR_ACTION_MAX),
							 Eq (testing_values::TITLEBAR_ACTION_MENU),
							 Eq (testing_values::TITLEBAR_ACTION_SHADE)));

    gwd_settings_storage_update_titlebar_actions (storage);
}

class GWDSettingsStorageGSettingsFactoryWrapper :
    public GWDSettingsStorageFactoryWrapperInterface
{
    public:

	virtual void SetUp (GWDSettingsWritable *writable)
	{
	    gsliceEnv.SetUpEnv ();
	    gsettingsEnv.SetUpEnv (MOCK_PATH);

	    mStorage.reset (gwd_settings_storage_new (writable, FALSE),
			    boost::bind (gwd_settings_storage_unref, _1));

	    /* We do not need to keep a reference to these */
	    mGWDSettings = gwd_get_org_compiz_gwd_settings (GetStorage ());
	    mMetacitySettings = gwd_get_org_gnome_metacity_settings (GetStorage ());
	    mDesktopSettings = gwd_get_org_gnome_desktop_wm_preferences_settings (GetStorage ());
	    mMarcoSettings = gwd_get_org_mate_marco_general_settings (GetStorage ());
	}

	virtual GWDSettingsStorage * GetStorage ()
	{
	    return mStorage.get ();
	}

	virtual void SetUseTooltips (gboolean useTooltips)
	{
	    g_settings_set_boolean (mGWDSettings, ORG_COMPIZ_GWD_KEY_USE_TOOLTIPS, useTooltips);
	}

	virtual void SetBlur (const std::string &blurType)
	{
	    g_settings_set_string (mGWDSettings, ORG_COMPIZ_GWD_KEY_BLUR_TYPE, blurType.c_str ());
	}

	virtual void SetOpacity (gdouble activeOpacity,
				 gdouble inactiveOpacity,
				 gboolean activeShadeOpacity,
				 gboolean inactiveShadeOpacity)
	{
	    g_settings_set_double (mGWDSettings, ORG_COMPIZ_GWD_KEY_METACITY_THEME_ACTIVE_OPACITY, activeOpacity);
	    g_settings_set_double (mGWDSettings, ORG_COMPIZ_GWD_KEY_METACITY_THEME_INACTIVE_OPACITY, inactiveOpacity);
	    g_settings_set_boolean (mGWDSettings, ORG_COMPIZ_GWD_KEY_METACITY_THEME_ACTIVE_SHADE_OPACITY, activeShadeOpacity);
	    g_settings_set_boolean (mGWDSettings, ORG_COMPIZ_GWD_KEY_METACITY_THEME_INACTIVE_SHADE_OPACITY, inactiveShadeOpacity);
	}

	virtual void SetMetacityTheme (gboolean useMetacityTheme,
				       const std::string &metacityTheme)
	{
	    g_settings_set_boolean (mGWDSettings, ORG_COMPIZ_GWD_KEY_USE_METACITY_THEME, useMetacityTheme);
	    g_settings_set_string (mDesktopSettings, ORG_GNOME_DESKTOP_WM_PREFERENCES_THEME, metacityTheme.c_str ());
	    g_settings_set_string (mMarcoSettings, ORG_MATE_MARCO_GENERAL_THEME, metacityTheme.c_str ());
	}

	virtual void SetButtonLayout (const std::string &buttonLayout)
	{
	    g_settings_set_string (mDesktopSettings,
				   ORG_GNOME_DESKTOP_WM_PREFERENCES_BUTTON_LAYOUT,
				   buttonLayout.c_str ());
	    g_settings_set_string (mMarcoSettings,
				   ORG_MATE_MARCO_GENERAL_BUTTON_LAYOUT,
				   buttonLayout.c_str ());
	}

	virtual void SetFont (gboolean useSystemFont, const std::string &titlebarFont)
	{
	    g_settings_set_boolean (mDesktopSettings,
				    ORG_GNOME_DESKTOP_WM_PREFERENCES_TITLEBAR_USES_SYSTEM_FONT,
				    useSystemFont);
	    g_settings_set_string (mDesktopSettings,
				   ORG_GNOME_DESKTOP_WM_PREFERENCES_TITLEBAR_FONT,
				   titlebarFont.c_str ());
	    g_settings_set_boolean (mMarcoSettings,
				    ORG_MATE_MARCO_GENERAL_TITLEBAR_USES_SYSTEM_FONT,
				    useSystemFont);
	    g_settings_set_string (mMarcoSettings,
				   ORG_MATE_MARCO_GENERAL_TITLEBAR_FONT,
				   titlebarFont.c_str ());
	}

	virtual void SetTitlebarActions (const std::string &doubleClickAction,
					 const std::string &middleClickAction,
					 const std::string &rightClickAction,
					 const std::string &mouseWheelAction)
	{
	    std::string translatedDC (doubleClickAction);
	    std::string translatedMC (middleClickAction);
	    std::string translatedRC (rightClickAction);

	    boost::replace_all (translatedDC, "_", "-");
	    boost::replace_all (translatedMC, "_", "-");
	    boost::replace_all (translatedRC, "_", "-");

	    g_settings_set_string (mDesktopSettings,
				   ORG_GNOME_DESKTOP_WM_PREFERENCES_ACTION_DOUBLE_CLICK_TITLEBAR,
				   translatedDC.c_str ());
	    g_settings_set_string (mDesktopSettings,
				   ORG_GNOME_DESKTOP_WM_PREFERENCES_ACTION_MIDDLE_CLICK_TITLEBAR,
				   translatedMC.c_str ());
	    g_settings_set_string (mDesktopSettings,
				   ORG_GNOME_DESKTOP_WM_PREFERENCES_ACTION_RIGHT_CLICK_TITLEBAR,
				   translatedRC.c_str ());
	    g_settings_set_string (mMarcoSettings,
				   ORG_MATE_MARCO_GENERAL_ACTION_DOUBLE_CLICK_TITLEBAR,
				   translatedDC.c_str ());
	    g_settings_set_string (mMarcoSettings,
				   ORG_MATE_MARCO_GENERAL_ACTION_MIDDLE_CLICK_TITLEBAR,
				   translatedMC.c_str ());
	    g_settings_set_string (mMarcoSettings,
				   ORG_MATE_MARCO_GENERAL_ACTION_RIGHT_CLICK_TITLEBAR,
				   translatedRC.c_str ());
	    g_settings_set_string (mGWDSettings,
				   ORG_COMPIZ_GWD_KEY_MOUSE_WHEEL_ACTION,
				   mouseWheelAction.c_str ());
	}

	virtual void TearDown ()
	{
	    mStorage.reset ();
	    mGWDSettings = NULL;
	    mMetacitySettings = NULL;
	    mDesktopSettings = NULL;
	    mMarcoSettings = NULL;
	    gsettingsEnv.TearDownEnv ();
	    gsliceEnv.TearDownEnv ();
	}

    private:

	GSettings			       *mGWDSettings;
	GSettings			       *mMetacitySettings;
	GSettings			       *mDesktopSettings;
	GSettings			       *mMarcoSettings;
	boost::shared_ptr <GWDSettingsStorage> mStorage;
	CompizGLibGSliceOffEnv                 gsliceEnv;
	CompizGLibGSettingsMemoryBackendTestingEnv gsettingsEnv;
};

INSTANTIATE_TEST_CASE_P (GSettingsStorageUpdates, GWDSettingsTestStorageUpdates,
			 ::testing::Values (boost::shared_ptr <GWDSettingsStorageFactoryWrapperInterface> (new GWDSettingsStorageGSettingsFactoryWrapper ())));
